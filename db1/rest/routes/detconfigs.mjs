import express from "express";
import { Binary } from "mongodb";
import multer from "multer";
import archiver from "archiver";
import stream from "stream";
import fs from "fs";

import db from "../db/conn.mjs";

const router = express.Router();

const upload = multer({
  storage: multer.memoryStorage(),
  limits: {
    fileSize: 10 * 1024 * 1024, // 10MB per file (MongoDB BSON limit is 16MB)
    files: 1000,
  },
});

const singleUpload = multer({ dest: "uploads/" });

// --------------------------------------------------------------
// -- Upload a single file to MongoDB detconfigs
router.post("/upload", upload.single("file"), async (req, res) => {
  console.log("detconfigs upload req.body:" + JSON.stringify(req.body));
  if (!req.file || !req.body.tag) {
    return res.status(400).send("File and tag are required");
  }

  let filesCollection = db.collection("detconfigs");

  try {
    const fileData = {
      tag: req.body.tag,
      filename: req.file.originalname,
      content: req.file.buffer,
    };

    const result = await filesCollection.insertOne(fileData);
    res.status(200).send(`File uploaded successfully with ID: ${result.insertedId}`);
  } catch (err) {
    res.status(500).send("Error uploading file: " + err.message);
  }
});

// --------------------------------------------------------------
// -- Upload multiple files to MongoDB detconfigs
router.post("/uploadMany", upload.array("file"), async (req, res) => {
  console.log("detconfigs uploadMany:", {
    files: req.files ? req.files.length : 0,
    tag: req.body.tag,
    bodyKeys: Object.keys(req.body),
  });

  if (!req.files || !req.body.tag) {
    return res.status(400).send("Files and tag are required");
  }

  let filesCollection = db.collection("detconfigs");

  try {
    const fileDocs = req.files.map((file) => ({
      tag: req.body.tag,
      filename: file.originalname,
      content: file.buffer,
      uploadDate: new Date(),
      size: file.size,
    }));

    console.log(`Attempting to insert ${fileDocs.length} files for tag ${req.body.tag}`);
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

  let filesCollection = db.collection("detconfigs");

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

    files.forEach((file) => {
      const bufferStream = new stream.PassThrough();
      // -- note: .buffer is absolutely essential
      bufferStream.end(file.content.buffer);
      archive.append(bufferStream, { name: file.filename });
    });

    archive.on("error", (err) => {
      throw err;
    });

    archive.finalize();
  } catch (err) {
    res.status(500).send("Error retrieving files: " + err.message);
  }
});

// ----------------------------------------------------------------------
// curl -X POST -F "tag=j1" -F "filename=j1/root.json" -F "file=@j1/root.json" \
//   http://localhost:5050/detconfigs/uploadJSON
router.post("/uploadJSON", singleUpload.single("file"), async (req, res) => {
  try {
    let collection = db.collection("detconfigs");

    const { tag, filename } = req.body;
    const filePath = req.file.path;

    const fileContent = fs.readFileSync(filePath);

    await collection.insertOne({
      tag,
      filename,
      content: Binary(fileContent),
    });

    fs.unlinkSync(filePath);

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
    let collection = db.collection("detconfigs");

    const fileDocuments = await collection.find({ tag: req.params.tag }).toArray();
    if (fileDocuments.length > 0) {
      const fileDocument = fileDocuments[fileDocuments.length - 1];
      if (fileDocument) {
        const fileContentBuffer = fileDocument.content.buffer;
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
    let detconfigsCollection = await db.collection("detconfigs");
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
    let detconfigsCollection = await db.collection("detconfigs");
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
// -- Delete all documents in detconfigs for a given tag
async function deleteDetconfigsByTag(req, res) {
  const tag = req.query.tag;

  if (!tag) {
    return res.status(400).json({ success: false, message: "Tag parameter is required" });
  }

  try {
    let detconfigsCollection = await db.collection("detconfigs");
    const result = await detconfigsCollection.deleteMany({ tag });

    if (result.deletedCount === 0) {
      return res.status(404).json({
        success: false,
        message: `No documents found with tag: ${tag}`,
      });
    }

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
