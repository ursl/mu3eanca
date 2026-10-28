import express from "express";
import { GridFSBucket, ObjectId } from "mongodb";
import path from "path";
import { dirname } from "path";
import { fileURLToPath } from "url";

import db from "../db/conn.mjs";

const __dirname = dirname(fileURLToPath(import.meta.url));

const PAYLOAD_GRIDFS_BUCKET = "payloadBlobs";
// Same cutoff as cdbUtil.hh kMaxInlinePayloadB64Chars. Larger base64 BLOBs
// are stored in GridFS and the document keeps blobStorage/blobGridFsId/blobSize.
const MAX_INLINE_PAYLOAD_B64 = 10 * 1024 * 1024;
const CDB_COLLECTIONS = new Set([
  "globaltags",
  "tags",
  "payloads",
  "configs",
  "runrecords",
]);

/**
 * When several `tags` documents share the same `tag`, prefer one with a non-empty
 * top-level string `comment`, then newest _id. History[].comment is ignored for this
 * (cdb.html tag table shows only top-level comment).
 */
const TAG_TOP_LEVEL_COMMENT_STAGES = [
  {
    $addFields: {
      __commentLen: {
        $cond: [
          { $eq: [{ $type: "$comment" }, "string"] },
          { $strLenCP: "$comment" },
          0,
        ],
      },
    },
  },
  {
    $addFields: {
      __hasComment: { $cond: [{ $gt: ["$__commentLen", 0] }, 1, 0] },
    },
  },
];

/** If payload doc stores BLOB in GridFS, download and set BLOB (base64) for API clients. */
async function mergePayloadBlobFromGridFS(database, result) {
  if (!result || result.blobStorage !== "gridfs" || !result.blobGridFsId) {
    return result;
  }
  const bucket = new GridFSBucket(database, { bucketName: PAYLOAD_GRIDFS_BUCKET });
  const oid = new ObjectId(result.blobGridFsId);
  const chunks = [];
  const stream = bucket.openDownloadStream(oid);
  await new Promise((resolve, reject) => {
    stream.on("data", (chunk) => chunks.push(chunk));
    stream.on("error", reject);
    stream.on("end", resolve);
  });
  const buf = Buffer.concat(chunks);
  return { ...result, BLOB: buf.toString("base64") };
}

const router = express.Router();

// --------------------------------------------------------------
// -- Get a single runrecord
router.get("/findOne/runrecords/:id", async (req, res) => {
  let runno = parseInt(req.params.id);
  console.log("serving /findOne/runrecords/" + req.params.id + " from " + req.ip);

  let collection = await db.collection("runrecords");

  let query = {"BOR.Run number": runno};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});

// ----------------------------------------------------------------------
// -- Get all run numbers
router.get("/findAll/runNumbers", async (req, res) => {
    console.log("serving from CDB /findAll/runNumbers");
    let collection = await db.collection("runrecords");
    let results = await collection.find({})
      .toArray();

    let runNumbers = results.map(record => record.BOR["Run number"]);
    res.send(runNumbers).status(200);
  
  });




// --------------------------------------------------------------
// -- Default CDB landing page 
router.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "../public/cdb.html"));
});


// --------------------------------------------------------------
// -- Get a single global tag
router.get("/findOne/globaltags/:id", async (req, res) => {
  console.log("serving /findOne/globaltags/" + req.params.id);
  let collection = await db.collection("globaltags");
  let query = {gt: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// --------------------------------------------------------------
// -- Get a single tag/IOV
router.get("/findOne/tags/:id", async (req, res) => {
  console.log("serving /findOne/tags/" + req.params.id);
  let collection = await db.collection("tags");
  const rows = await collection
    .aggregate([
      { $match: { tag: req.params.id } },
      ...TAG_TOP_LEVEL_COMMENT_STAGES,
      { $sort: { __hasComment: -1, _id: -1 } },
      { $limit: 1 },
      {
        $project: {
          __commentLen: 0,
          __hasComment: 0,
        },
      },
    ])
    .toArray();
  const result = rows[0];
  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// --------------------------------------------------------------
// -- Get a single payload
router.get("/findOne/payloads/:id", async (req, res) => {
  console.log("serving /findOne/payloads/" + req.params.id);
  let collection = await db.collection("payloads");
  let query = {hash: req.params.id};
  let result = await collection.findOne(query);

  if (!result) {
    return res.status(404).send("Not found");
  }
  try {
    result = await mergePayloadBlobFromGridFS(db, result);
    return res.status(200).send(result);
  } catch (err) {
    console.error("findOne/payloads GridFS merge failed:", err);
    return res.status(500).send("GridFS read failed: " + err.message);
  }
});


// --------------------------------------------------------------
// -- Get a single configuration 
router.get("/findOne/configs/:id", async (req, res) => {
  console.log("serving /findOne/configs/" + req.params.id);
  let collection = await db.collection("configs");
  let query = {cfgHash: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// --------------------------------------------------------------
// -- Get all globaltags
router.get("/findAll/globaltags", async (req, res) => {
  console.log("serving /findAll/globaltags");
  let collection = await db.collection("globaltags");
  let results = await collection.find({})
    .limit(50)
    .toArray();

  res.send(results).status(200);
});


// --------------------------------------------------------------
// -- Get tags for a specific globaltag (slim rows for cdb.html).
// NOTE: cdbSummaryGT / cdbRest::getTagComment use GET /findOne/tags/:id per tag, not this route.
// Response always includes string fields tag, description, comment, iovs (comment may be "").
router.get("/findTagsByGlobaltag/:globaltag", async (req, res) => {
  console.log("serving /findTagsByGlobaltag/" + req.params.globaltag);
  
  try {
    // First get the globaltag document to get its tags array
    let globaltagsCollection = await db.collection("globaltags");
    let globaltag = await globaltagsCollection.findOne({gt: req.params.globaltag});
    
    if (!globaltag || !globaltag.tags) {
      console.log("No globaltag found or no tags array:", req.params.globaltag);
      return res.send([]).status(200);
    }
    
    console.log("Found globaltag with tags:", globaltag.tags);
    
    // Then get distinct tags that are in the globaltag's tags array
    let tagsCollection = await db.collection("tags");
    // Multiple Mongo docs can share the same tag (re-sync). Prefer a non-empty
    // top-level `comment`, then newest _id (History is not used for the table text).
    let results = await tagsCollection.aggregate([
      { $match: { tag: { $in: globaltag.tags } } },
      ...TAG_TOP_LEVEL_COMMENT_STAGES,
      { $sort: { tag: 1, __hasComment: -1, _id: -1 } },
      {
        $group: {
          _id: "$tag",
          tag: { $first: "$tag" },
          description: { $first: "$description" },
          comment: { $first: "$comment" },
          iovs: { $first: "$iovs" },
        },
      },
      {
        $project: {
          _id: 0,
          tag: 1,
          description: 1,
          comment: 1,
          iovs: 1,
        },
      },
    ]).toArray();

    // Plain objects + always set `comment` so JSON always contains the key (driver may omit
    // undefined/null fields from aggregation output; browser console then hides `comment`).
    results = results.map((r) => {
      const c = r.comment != null && r.comment !== undefined ? String(r.comment) : "";
      return {
        tag: r.tag != null ? String(r.tag) : "",
        description: r.description != null ? String(r.description) : "",
        comment: c,
        iovs: Array.isArray(r.iovs) ? r.iovs : [],
      };
    });

    console.log("Found distinct tags:", results.length);
    console.log("First tag (if any):", results.length > 0 ? JSON.stringify(results[0]) : "none");
    if (process.env.CDB_DBX) {
      for (const r of results) {
        const c = r.comment != null ? String(r.comment) : "";
        console.log(
          "CDB_DBX findTagsByGlobaltag tag=",
          r.tag,
          " commentLen=",
          c.length,
          c.length ? " preview=" + JSON.stringify(c.slice(0, 120)) : ""
        );
      }
    }
    
    res.status(200).json(results);
  } catch (error) {
    console.error("Error in findTagsByGlobaltag:", error);
    res.status(500).send({ error: error.message });
  }
});

// --------------------------------------------------------------
// -- Get payloads for a specific tag
router.get("/findPayloadsByTag/:tag", async (req, res) => {
  console.log("serving /findPayloadsByTag/" + req.params.tag);
  
  try {
    // First get the tag document to get its IOVs array
    let tagsCollection = await db.collection("tags");
    let tag = await tagsCollection.findOne({tag: req.params.tag});
    
    if (!tag || !tag.iovs) {
      console.log("No tag found or no IOVs array:", req.params.tag);
      return res.send({ tag: req.params.tag, iovs: [], payloads: [] }).status(200);
    }
    
    console.log("Found tag with IOVs:", tag.iovs);
    
    // Then get all payloads for this tag
    let payloadsCollection = await db.collection("payloads");
    let query = {hash: { $regex: "^tag_" + req.params.tag + "_iov_" }};
    console.log("MongoDB query for payloads:", JSON.stringify(query));
    
    // Add a limit to prevent memory issues with very large datasets
    // Default limit is 1000, but can be overridden with query parameter
    const limit = parseInt(req.query.limit) || 1000;
    
    // Get count first to inform the client
    const totalCount = await payloadsCollection.countDocuments(query);
    console.log("Total payloads found:", totalCount);
    
    // Fetch payloads with limit and only essential fields to reduce response size
    let results = await payloadsCollection.find(query)
      .project({ hash: 1, date: 1, comment: 1, schema: 1, _id: 0 }) // Only include essential fields
      .sort({ date: -1 }) // Sort by date descending
      .limit(limit)
      .toArray();
    
    console.log("Returning payloads:", results.length, "of", totalCount);
    
    // Add the IOVs array from the tag document to the response
    let response = {
      tag: tag.tag,
      iovs: tag.iovs,
      payloads: results,
      totalCount: totalCount,
      returnedCount: results.length,
      hasMore: totalCount > limit
    };
    
    res.send(response).status(200);
  } catch (error) {
    console.error("Error in findPayloadsByTag:", error);
    console.error("Error stack:", error.stack);
    res.status(500).json({ 
      error: error.message,
      details: error.toString(),
      stack: process.env.NODE_ENV === 'development' ? error.stack : undefined
    });
  }
});

// --------------------------------------------------------------
// -- Debug route to check payloads collection
router.get("/debug/payloads", async (req, res) => {
  console.log("Debug: checking payloads collection");
  
  try {
    let payloadsCollection = await db.collection("payloads");
    let results = await payloadsCollection.find({}).limit(5).toArray();
    
    console.log("Found payloads:", results.length);
    console.log("Sample payloads:", JSON.stringify(results, null, 2));
    
    res.send({
      count: await payloadsCollection.countDocuments(),
      sample: results
    }).status(200);
  } catch (error) {
    console.error("Error in debug/payloads:", error);
    res.status(500).send({ error: error.message });
  }
});

function cdbTimeStamp() {
  const d = new Date();
  const p = (n) => String(n).padStart(2, "0");
  return `${d.getFullYear()}/${p(d.getMonth() + 1)}/${p(d.getDate())} ${p(d.getHours())}:${p(d.getMinutes())}:${p(d.getSeconds())}`;
}

function payloadBucket() {
  return new GridFSBucket(db, { bucketName: PAYLOAD_GRIDFS_BUCKET });
}

function uploadPayloadBlob(filename, buffer) {
  return new Promise((resolve, reject) => {
    const uploadStream = payloadBucket().openUploadStream(filename);
    uploadStream.on("error", reject);
    uploadStream.on("finish", () => resolve(uploadStream.id));
    uploadStream.end(buffer);
  });
}

async function deleteGridFsIfPresent(doc) {
  if (!doc || doc.blobStorage !== "gridfs" || !doc.blobGridFsId) return;
  const hashInfo = typeof doc.hash === "string" ? doc.hash : "?";
  try {
    await payloadBucket().delete(new ObjectId(doc.blobGridFsId));
    console.log(`sync: deleted GridFS blob id=${doc.blobGridFsId} hash=${hashInfo}`);
  } catch (err) {
    console.error(`sync: GridFS delete failed id=${doc.blobGridFsId} hash=${hashInfo}:`, err.message);
  }
}

async function offloadLargePayloadBlob(doc) {
  if (typeof doc.BLOB !== "string" || doc.BLOB.length <= MAX_INLINE_PAYLOAD_B64) {
    return doc;
  }
  const buf = Buffer.from(doc.BLOB, "base64");
  const hashStr = typeof doc.hash === "string" ? doc.hash : "payload";
  const id = await uploadPayloadBlob(hashStr, buf);
  delete doc.BLOB;
  doc.blobStorage = "gridfs";
  doc.blobGridFsId = id.toString();
  doc.blobSize = buf.length;
  return doc;
}

function escapeRegex(s) {
  return s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

async function deleteMatching(collectionName, filter) {
  const col = db.collection(collectionName);
  const docs = await col.find(filter).toArray();
  let n = 0;
  for (const doc of docs) {
    await deleteGridFsIfPresent(doc);
    const r = await col.deleteOne({ _id: doc._id });
    n += r.deletedCount || 0;
  }
  return n;
}

// --------------------------------------------------------------
// -- Insert one JSON document. X-Cdb-File is the on-disk basename.
router.post("/insert/:collection", async (req, res) => {
  const collection = req.params.collection;
  if (!CDB_COLLECTIONS.has(collection)) {
    return res.status(400).send("unknown collection " + collection);
  }
  const fileBase = String(req.get("x-cdb-file") || "");
  if (!fileBase) return res.status(400).send("missing X-Cdb-File");
  if (!req.body || typeof req.body !== "object" || Array.isArray(req.body)) {
    return res.status(400).send("JSON object required");
  }

  const doc = req.body;
  if (collection === "globaltags") {
    if (typeof doc.gt !== "string" || doc.gt !== fileBase) {
      return res.status(400).send(
        `filename "${fileBase}" does not match "gt" "${doc.gt ?? ""}"`
      );
    }
  }
  if (collection === "runrecords") {
    if (!Object.prototype.hasOwnProperty.call(doc, "EOR")) {
      return res.status(400).send("EOR not found");
    }
    if (Array.isArray(doc.EOR) && doc.EOR.length > 1) {
      return res.status(400).send("multiple EORs");
    }
  }

  delete doc._id;
  doc.History = [{ date: cdbTimeStamp(), comment: "Database entry inserted" }];
  try {
    if (collection === "payloads") await offloadLargePayloadBlob(doc);
  } catch (err) {
    console.error("GridFS offload failed:", err);
    return res.status(500).send("GridFS offload failed: " + err.message);
  }

  const result = await db.collection(collection).insertOne(doc);
  console.log(`inserted ${collection}/${fileBase} _id=${result.insertedId}`);
  return res.status(201).json({
    insertedId: String(result.insertedId),
    file: fileBase,
  });
});

router.delete("/globaltags/:gt", async (req, res) => {
  const n = await deleteMatching("globaltags", { gt: req.params.gt });
  console.log(`delete globaltags gt=${req.params.gt} count=${n}`);
  return res.status(200).json({ deleted: n });
});

router.delete("/tags/:tag", async (req, res) => {
  const n = await deleteMatching("tags", { tag: req.params.tag });
  console.log(`delete tags tag=${req.params.tag} count=${n}`);
  return res.status(200).json({ deleted: n });
});

router.delete("/payloads/hash/:hash", async (req, res) => {
  const n = await deleteMatching("payloads", { hash: req.params.hash });
  console.log(`delete payloads hash=${req.params.hash} count=${n}`);
  return res.status(200).json({ deleted: n });
});

router.delete("/payloads", async (req, res) => {
  const tagDir = req.query.tagDir;
  if (!tagDir) return res.status(400).send("tagDir required");
  const rx = "^tag_" + escapeRegex(String(tagDir)) + "_iov_";
  const n = await deleteMatching("payloads", { hash: { $regex: rx } });
  console.log(`delete payloads for tag dir ${tagDir} count=${n}`);
  return res.status(200).json({ deleted: n });
});

// Legacy clear: regex (case-insensitive) on gt, tag, hash, and cfgHash,
// or every document when all=1.
router.delete("/clear/:collection", async (req, res) => {
  const collection = req.params.collection;
  if (!CDB_COLLECTIONS.has(collection)) {
    return res.status(400).send("unknown collection " + collection);
  }
  let n = 0;
  if (req.query.all === "1") {
    n = await deleteMatching(collection, {});
  } else if (req.query.pat) {
    const pat = String(req.query.pat);
    for (const field of ["gt", "tag", "hash", "cfgHash"]) {
      n += await deleteMatching(collection, { [field]: { $regex: pat, $options: "i" } });
    }
  } else {
    return res.status(400).send("pat or all=1 required");
  }
  console.log(`clear ${collection} count=${n}`);
  return res.status(200).json({ deleted: n });
});

console.log(
  "[cdb routes] loaded: findTagsByGlobaltag returns { tag, description, comment, iovs } (comment always present)"
);

export default router;
