import express from "express";
import multer from "multer";
import { GridFSBucket, ObjectId } from "mongodb";

import db from "../db/conn.mjs";

const router = express.Router();

const COLLECTION = "detcal";
const GRIDFS_BUCKET = "detcalBlobs";

// Always GridFS (parallel to payloads / payloadBlobs). Metadata lives in `detcal`;
// file bytes live in GridFS — no need to split the collection for Mongo.
const upload = multer({
  storage: multer.memoryStorage(),
  limits: {
    fileSize: 100 * 1024 * 1024, // 100MB; GridFS has no 16MB BSON limit
  },
});

let detcalBucket = null;
function getBucket() {
  if (!db?.databaseName) return null;
  if (!detcalBucket) {
    detcalBucket = new GridFSBucket(db, { bucketName: GRIDFS_BUCKET });
  }
  return detcalBucket;
}

let indexReady = false;
async function ensureIndexes() {
  if (indexReady) return;
  const collection = db.collection(COLLECTION);
  await collection.createIndex({ name: 1, run: 1 }, { unique: true });
  indexReady = true;
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
    // Missing file is fine when replacing a broken/partial doc
    if (err?.code !== "ENOENT" && err?.message && !/FileNotFound/i.test(err.message)) {
      throw err;
    }
  }
}

async function findDetcal(name, run, { exact = false } = {}) {
  const collection = db.collection(COLLECTION);
  if (exact) {
    return collection.findOne({ name, run });
  }
  return collection
    .find({ name, run: { $lte: run } })
    .sort({ run: -1 })
    .limit(1)
    .next();
}

function metaResponse(doc) {
  return {
    name: doc.name,
    run: doc.run,
    filename: doc.filename,
    comment: doc.comment || "",
    date: doc.date,
    blobStorage: doc.blobStorage,
    blobGridFsId: String(doc.blobGridFsId),
  };
}

// ----------------------------------------------------------------------
// Upload a detector-calibration file into Mongo + GridFS (any content; no payload validation).
//
// curl -X POST -F "name=pixelpedestal" -F "run=7559" -F "file=@pedestal.json" \
//   http://localhost:5050/detcal/upload
// curl -X POST -F "name=pixelpedestal" -F "run=7559" \
//   -F "file=@pedestal.root" -F "replace=1" http://localhost:5050/detcal/upload
router.post("/upload", upload.single("file"), async (req, res) => {
  try {
    if (!req.file) {
      return res.status(400).send("No file uploaded (form field 'file')");
    }

    const name = (req.body.name || "").trim();
    const run = parseInt(req.body.run, 10);
    if (!name) {
      return res.status(400).send("Missing detcal 'name'");
    }
    if (!Number.isFinite(run) || run < 0) {
      return res.status(400).send("Missing or invalid 'run' (non-negative integer)");
    }

    const bucket = getBucket();
    if (!bucket) {
      return res.status(503).send("MongoDB file storage unavailable");
    }

    await ensureIndexes();
    const collection = db.collection(COLLECTION);
    const replace =
      req.body.replace === "1" ||
      req.body.replace === "true" ||
      req.query.replace === "1";

    const existing = await collection.findOne({ name, run });
    if (existing && !replace) {
      return res
        .status(409)
        .send(`Detcal '${name}' for run ${run} already exists (pass replace=1 to overwrite)`);
    }

    const filename = req.file.originalname || `${name}_${run}`;
    const gridFsId = await uploadToGridFS(bucket, filename, req.file.buffer, {
      name,
      run,
    });

    if (existing?.blobGridFsId) {
      await deleteGridFsFile(bucket, existing.blobGridFsId);
    }

    const doc = {
      name,
      run,
      filename,
      comment: req.body.comment || "",
      date: new Date().toISOString(),
      blobStorage: "gridfs",
      blobGridFsId: gridFsId,
    };

    if (existing) {
      await collection.replaceOne({ _id: existing._id }, doc);
    } else {
      await collection.insertOne(doc);
    }

    res.status(200).json({
      message: existing ? "Detcal replaced" : "Detcal uploaded",
      detcal: metaResponse(doc),
    });
  } catch (error) {
    console.error("detcal/upload:", error);
    res.status(500).send("Error uploading detcal: " + error.message);
  }
});

// ----------------------------------------------------------------------
// Metadata for the most recent detcal with run <= :run (or exact with ?exact=1).
// curl http://localhost:5050/detcal/pixelpedestal/7559/meta
router.get("/:name/:run/meta", async (req, res) => {
  try {
    const name = req.params.name;
    const run = parseInt(req.params.run, 10);
    if (!Number.isFinite(run)) {
      return res.status(400).send("Invalid run");
    }
    await ensureIndexes();
    const exact = req.query.exact === "1" || req.query.exact === "true";
    const doc = await findDetcal(name, run, { exact });
    if (!doc) {
      return res.status(404).send("Not found");
    }
    res.status(200).json(metaResponse(doc));
  } catch (error) {
    console.error("detcal meta:", error);
    res.status(500).send("Error reading detcal metadata: " + error.message);
  }
});

// ----------------------------------------------------------------------
// Download detcal bytes for most recent run <= :run (IOV-style, like C++ whichIOV).
// Use ?exact=1 for an exact run match.
//
// curl -OJ "http://localhost:5050/detcal/pixelpedestal/7559"
// curl -OJ "http://localhost:5050/detcal/pixelpedestal/7559?exact=1"
router.get("/:name/:run", async (req, res) => {
  try {
    const name = req.params.name;
    const run = parseInt(req.params.run, 10);
    if (!Number.isFinite(run)) {
      return res.status(400).send("Invalid run");
    }

    const bucket = getBucket();
    if (!bucket) {
      return res.status(503).send("MongoDB file storage unavailable");
    }

    await ensureIndexes();
    const exact = req.query.exact === "1" || req.query.exact === "true";
    const doc = await findDetcal(name, run, { exact });
    if (!doc) {
      return res.status(404).send("Not found");
    }
    if (doc.blobStorage !== "gridfs" || !doc.blobGridFsId) {
      return res.status(500).send("Detcal has no GridFS blob");
    }

    const oid = new ObjectId(doc.blobGridFsId);
    const files = await bucket.find({ _id: oid }).toArray();
    if (files.length === 0) {
      return res.status(404).send("GridFS file not found");
    }

    res.setHeader("Content-Disposition", `attachment; filename="${doc.filename}"`);
    res.setHeader("X-Detcal-Name", doc.name);
    res.setHeader("X-Detcal-Run", String(doc.run));
    res.setHeader("Content-Length", files[0].length);

    bucket.openDownloadStream(oid).pipe(res);
  } catch (error) {
    console.error("detcal download:", error);
    if (!res.headersSent) {
      res.status(500).send("Error downloading detcal: " + error.message);
    }
  }
});

// ----------------------------------------------------------------------
// List stored runs for a detcal name (newest first).
// curl http://localhost:5050/detcal/pixelpedestal
router.get("/:name", async (req, res) => {
  try {
    await ensureIndexes();
    const collection = db.collection(COLLECTION);
    const rows = await collection
      .find({ name: req.params.name })
      .project({ blobGridFsId: 0 })
      .sort({ run: -1 })
      .toArray();
    res.status(200).json(
      rows.map((doc) => ({
        name: doc.name,
        run: doc.run,
        filename: doc.filename,
        comment: doc.comment || "",
        date: doc.date,
      })),
    );
  } catch (error) {
    console.error("detcal list:", error);
    res.status(500).send("Error listing detcal: " + error.message);
  }
});

// ----------------------------------------------------------------------
// Delete exact (name, run). Removes metadata + GridFS blob.
// curl -X DELETE http://localhost:5050/detcal/pixelpedestal/7559
router.delete("/:name/:run", async (req, res) => {
  try {
    const name = req.params.name;
    const run = parseInt(req.params.run, 10);
    if (!Number.isFinite(run)) {
      return res.status(400).send("Invalid run");
    }

    const bucket = getBucket();
    if (!bucket) {
      return res.status(503).send("MongoDB file storage unavailable");
    }

    const collection = db.collection(COLLECTION);
    const doc = await collection.findOne({ name, run });
    if (!doc) {
      return res.status(404).send("Not found");
    }

    if (doc.blobGridFsId) {
      await deleteGridFsFile(bucket, doc.blobGridFsId);
    }
    await collection.deleteOne({ _id: doc._id });
    res.status(200).json({ message: "Detcal deleted", name, run });
  } catch (error) {
    console.error("detcal delete:", error);
    res.status(500).send("Error deleting detcal: " + error.message);
  }
});

export default router;
