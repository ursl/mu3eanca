import express from "express";
import { GridFSBucket, ObjectId } from "mongodb";
import path from "path";
import { dirname } from "path";
import { fileURLToPath } from "url";

import db from "../db/conn.mjs";

const __dirname = dirname(fileURLToPath(import.meta.url));

const PAYLOAD_GRIDFS_BUCKET = "payloadBlobs";

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

console.log(
  "[cdb routes] loaded: findTagsByGlobaltag returns { tag, description, comment, iovs } (comment always present)"
);

export default router;
