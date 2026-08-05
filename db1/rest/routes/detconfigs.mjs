import express from "express";
import { GridFSBucket, ObjectId } from "mongodb";
import multer from "multer";
import archiver from "archiver";
import stream from "stream";

import db from "../db/conn.mjs";

const router = express.Router();

const COLLECTION = "detconfigs";
// Parallel to detcal / detcalBlobs — separate GridFS bucket for config file bytes.
const GRIDFS_BUCKET = "detconfigBlobs";

const upload = multer({
  storage: multer.memoryStorage(),
  limits: {
    fileSize: 100 * 1024 * 1024, // 100MB; GridFS has no 16MB BSON limit
    files: 1000,
  },
});

let detconfigBucket = null;
function getBucket() {
  if (!db?.databaseName) return null;
  if (!detconfigBucket) {
    detconfigBucket = new GridFSBucket(db, { bucketName: GRIDFS_BUCKET });
  }
  return detconfigBucket;
}

function uploadToGridFS(bucket, filename, buffer, metadata) {
  return new Promise((resolve, reject) => {
    const uploadStream = bucket.openUploadStream(filename, { metadata });
    uploadStream.on("error", reject);
    uploadStream.on("finish", () => resolve(uploadStream.id));
    uploadStream.end(buffer);
  });
}

async function deleteGridFsFile(bucket, id) {
  if (!id) return;
  try {
    await bucket.delete(new ObjectId(id));
  } catch (err) {
    if (err?.code !== "ENOENT" && err?.message && !/FileNotFound/i.test(err.message)) {
      throw err;
    }
  }
}

/** Load file bytes: GridFS (new) or legacy inline `content` Binary/Buffer. */
async function readFileBuffer(bucket, doc) {
  if (doc.blobStorage === "gridfs" && doc.blobGridFsId) {
    if (!bucket) {
      throw new Error("MongoDB file storage unavailable");
    }
    const oid = new ObjectId(doc.blobGridFsId);
    const chunks = [];
    const downloadStream = bucket.openDownloadStream(oid);
    await new Promise((resolve, reject) => {
      downloadStream.on("data", (chunk) => chunks.push(chunk));
      downloadStream.on("error", reject);
      downloadStream.on("end", resolve);
    });
    return Buffer.concat(chunks);
  }

  // Legacy docs stored content inline in the detconfigs collection
  if (doc.content?.buffer) {
    return Buffer.from(doc.content.buffer);
  }
  if (Buffer.isBuffer(doc.content)) {
    return doc.content;
  }
  throw new Error(`No file bytes for ${doc.filename || doc._id}`);
}

// --------------------------------------------------------------
// -- Upload a single file (metadata in detconfigs, bytes in detconfigBlobs)
router.post("/upload", upload.single("file"), async (req, res) => {
  console.log("detconfigs upload req.body:" + JSON.stringify(req.body));
  if (!req.file || !req.body.tag) {
    return res.status(400).send("File and tag are required");
  }

  const bucket = getBucket();
  if (!bucket) {
    return res.status(503).send("MongoDB file storage unavailable");
  }

  try {
    const tag = req.body.tag;
    const filename = req.file.originalname;
    const blobGridFsId = await uploadToGridFS(bucket, filename, req.file.buffer, {
      tag,
      filename,
    });

    const filesCollection = db.collection(COLLECTION);
    const result = await filesCollection.insertOne({
      tag,
      filename,
      size: req.file.size,
      uploadDate: new Date(),
      blobStorage: "gridfs",
      blobGridFsId,
    });
    res.status(200).send(`File uploaded successfully with ID: ${result.insertedId}`);
  } catch (err) {
    res.status(500).send("Error uploading file: " + err.message);
  }
});

// --------------------------------------------------------------
// -- Upload multiple files
router.post("/uploadMany", upload.array("file"), async (req, res) => {
  console.log("detconfigs uploadMany:", {
    files: req.files ? req.files.length : 0,
    tag: req.body.tag,
    bodyKeys: Object.keys(req.body),
  });

  if (!req.files || !req.body.tag) {
    return res.status(400).send("Files and tag are required");
  }

  const bucket = getBucket();
  if (!bucket) {
    return res.status(503).send("MongoDB file storage unavailable");
  }

  const tag = req.body.tag;
  const uploadedIds = [];

  try {
    const fileDocs = [];
    for (const file of req.files) {
      const blobGridFsId = await uploadToGridFS(bucket, file.originalname, file.buffer, {
        tag,
        filename: file.originalname,
      });
      uploadedIds.push(blobGridFsId);
      fileDocs.push({
        tag,
        filename: file.originalname,
        size: file.size,
        uploadDate: new Date(),
        blobStorage: "gridfs",
        blobGridFsId,
      });
    }

    console.log(`Attempting to insert ${fileDocs.length} files for tag ${tag}`);
    const filesCollection = db.collection(COLLECTION);
    const result = await filesCollection.insertMany(fileDocs);
    console.log(`Successfully uploaded ${result.insertedCount} files`);

    res.status(200).json({
      success: true,
      message: "Files uploaded successfully",
      count: result.insertedCount,
      ids: result.insertedIds,
    });
  } catch (err) {
    console.error("Error in detconfigs/uploadMany:", err);
    for (const id of uploadedIds) {
      await deleteGridFsFile(bucket, id).catch(() => {});
    }
    res.status(500).json({
      success: false,
      message: "Error uploading files: " + err.message,
      error: err.toString(),
    });
  }
});

// --------------------------------------------------------------
// -- Download all files as a ZIP archive by tag
router.get("/downloadTag", async (req, res) => {
  const tag = req.query.tag;

  if (!tag) {
    return res.status(400).send("Tag parameter is required");
  }

  const bucket = getBucket();
  const filesCollection = db.collection(COLLECTION);

  try {
    const files = await filesCollection.find({ tag }).toArray();

    files.forEach((file) => {
      console.log("file: " + file.filename);
    });

    if (files.length === 0) {
      return res.status(404).send("No files found for the given tag");
    }

    const archive = archiver("zip", { zlib: { level: 9 } });
    res.setHeader("Content-Type", "application/zip");
    res.setHeader("Content-Disposition", `attachment; filename=${tag}.zip`);

    archive.pipe(res);

    for (const file of files) {
      const buf = await readFileBuffer(bucket, file);
      const bufferStream = new stream.PassThrough();
      bufferStream.end(buf);
      archive.append(bufferStream, { name: file.filename });
    }

    archive.on("error", (err) => {
      throw err;
    });

    await archive.finalize();
  } catch (err) {
    if (!res.headersSent) {
      res.status(500).send("Error retrieving files: " + err.message);
    }
  }
});

// ----------------------------------------------------------------------
// curl -X POST -F "tag=j1" -F "filename=j1/root.json" -F "file=@j1/root.json" \
//   http://localhost:5050/detconfigs/uploadJSON
router.post("/uploadJSON", upload.single("file"), async (req, res) => {
  try {
    if (!req.file || !req.body.tag) {
      return res.status(400).send("File and tag are required");
    }

    const bucket = getBucket();
    if (!bucket) {
      return res.status(503).send("MongoDB file storage unavailable");
    }

    const tag = req.body.tag;
    const filename = req.body.filename || req.file.originalname;
    const blobGridFsId = await uploadToGridFS(bucket, filename, req.file.buffer, {
      tag,
      filename,
    });

    const collection = db.collection(COLLECTION);
    await collection.insertOne({
      tag,
      filename,
      size: req.file.size,
      uploadDate: new Date(),
      blobStorage: "gridfs",
      blobGridFsId,
    });

    res.status(200).send("File uploaded successfully");
  } catch (error) {
    console.error(error);
    res.status(500).send("Error uploading file");
  }
});

// ----------------------------------------------------------------------
// curl http://localhost:5050/detconfigs/downloadJSON/j2 -o root.json
router.get("/downloadJSON/:tag", async (req, res) => {
  try {
    const bucket = getBucket();
    const collection = db.collection(COLLECTION);

    const fileDocuments = await collection.find({ tag: req.params.tag }).toArray();
    if (fileDocuments.length > 0) {
      const fileDocument = fileDocuments[fileDocuments.length - 1];
      if (fileDocument) {
        const fileContentBuffer = await readFileBuffer(bucket, fileDocument);
        const fileContentJson = JSON.parse(fileContentBuffer.toString("utf8"));
        res.json(fileContentJson);
      }
    } else {
      res.status(404).send("File not found");
    }
  } catch (error) {
    console.error(error);
    res.status(500).send("Error retrieving file");
  }
});

// --------------------------------------------------------------
// -- List distinct detconfigs tags (one per line, plain text) for CLI tools
//    curl -fsS "http://host:5050/detconfigs/detconfigTags"
router.get("/detconfigTags", async (req, res) => {
  try {
    let detconfigsCollection = await db.collection(COLLECTION);
    let results = await detconfigsCollection
      .aggregate([{ $group: { _id: "$tag" } }, { $sort: { _id: 1 } }])
      .toArray();
    const lines = results.map((r) => r._id).filter((t) => t != null && t !== "");
    res.type("text/plain; charset=utf-8").send(lines.join("\n") + (lines.length ? "\n" : ""));
  } catch (error) {
    console.error("Error in detconfigTags:", error);
    res.status(500).type("text/plain").send("Error: " + error.message + "\n");
  }
});

// --------------------------------------------------------------
// -- Get summary of detconfigs tags and their counts
router.get("/findAll/detconfigsSummary", async (req, res) => {
  try {
    let detconfigsCollection = await db.collection(COLLECTION);
    let results = await detconfigsCollection
      .aggregate([
        { $group: { _id: "$tag", count: { $sum: 1 } } },
        { $project: { _id: 0, tag: "$_id", count: 1 } },
        { $sort: { tag: 1 } },
      ])
      .toArray();
    res.send(results).status(200);
  } catch (error) {
    console.error("Error in detconfigsSummary:", error);
    res.status(500).send({ error: error.message });
  }
});

// --------------------------------------------------------------
// -- Delete all documents in detconfigs for a given tag (+ GridFS blobs)
async function deleteDetconfigsByTag(req, res) {
  const tag = req.query.tag;

  if (!tag) {
    return res.status(400).json({ success: false, message: "Tag parameter is required" });
  }

  try {
    const bucket = getBucket();
    const detconfigsCollection = db.collection(COLLECTION);
    const docs = await detconfigsCollection.find({ tag }).toArray();

    if (docs.length === 0) {
      return res.status(404).json({
        success: false,
        message: `No documents found with tag: ${tag}`,
      });
    }

    if (bucket) {
      for (const doc of docs) {
        if (doc.blobGridFsId) {
          await deleteGridFsFile(bucket, doc.blobGridFsId);
        }
      }
    }

    const result = await detconfigsCollection.deleteMany({ tag });

    res.json({
      success: true,
      message: `Successfully deleted ${result.deletedCount} documents for tag: ${tag}`,
      deletedCount: result.deletedCount,
    });
  } catch (error) {
    console.error("Error deleting detconfigs tag:", error);
    res.status(500).json({
      success: false,
      message: "Error deleting tag: " + error.message,
    });
  }
}

router.delete("/deleteDetconfigTag", deleteDetconfigsByTag);
// Legacy alias (ambiguous name vs conditions DB "tags" collection)
router.delete("/deleteTag", deleteDetconfigsByTag);

export default router;
