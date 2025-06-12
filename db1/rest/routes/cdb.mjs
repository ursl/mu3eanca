import express from "express";
import {MongoClient, Binary} from "mongodb";

import multer from "multer";
import archiver from "archiver";
import stream from "stream";
import fs  from "fs";
import path from "path";
import { dirname } from 'path';
import { fileURLToPath } from 'url';

// Configure multer for file uploads
const upload = multer({
    storage: multer.memoryStorage(),
    limits: {
        fileSize: 10 * 1024 * 1024, // 10MB per file (MongoDB limit is 16MB)
        files: 1000 // Allow up to 1000 files per upload
    }
});

// Multer setup to handle file uploads
const singleUpload = multer({ dest: 'uploads/' });

import db from "../db/conn.mjs";

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
    console.log("serving from CDB /findAll/runNumbers " + req.params.id);
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
  let query = {tag: req.params.id};
  let result = await collection.findOne(query);
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

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
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
  console.log("serving /findAll/globaltags/" + req.params.id);
  let collection = await db.collection("globaltags");
  let results = await collection.find({})
    .limit(50)
    .toArray();

  res.send(results).status(200);
});


// --------------------------------------------------------------
// -- Upload a single file to MongoDB
router.post('/upload', upload.single('file'), async (req, res) => {
    console.log("upload req.body:" + JSON.stringify(req.body));
    if (!req.file || !req.body.tag) {
      return res.status(400).send('File and tag are required');
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
        res.status(500).send('Error uploading file: ' + err.message);
    }
});


// --------------------------------------------------------------
// -- Upload multiple files to MongoDB
router.post('/uploadMany', upload.array('file'), async (req, res) => {
    console.log("Upload request received:", {
        files: req.files ? req.files.length : 0,
        tag: req.body.tag,
        bodyKeys: Object.keys(req.body)
    });

    if (!req.files || !req.body.tag) {
        console.log("Missing required fields:", {
            hasFiles: !!req.files,
            hasTag: !!req.body.tag
        });
        return res.status(400).send('Files and tag are required');
    }
    
    let filesCollection = db.collection("detconfigs");
    
    try {
        const fileDocs = req.files.map(file => ({
            tag: req.body.tag,
            filename: file.originalname,
            content: file.buffer,
            uploadDate: new Date(),
            size: file.size
        }));
        
        console.log(`Attempting to insert ${fileDocs.length} files for tag ${req.body.tag}`);
        const result = await filesCollection.insertMany(fileDocs);
        console.log(`Successfully uploaded ${result.insertedCount} files`);
        
        res.status(200).json({
            success: true,
            message: `Files uploaded successfully`,
            count: result.insertedCount,
            ids: result.insertedIds
        });
    } catch (err) {
        console.error('Error in uploadMany:', err);
        res.status(500).json({
            success: false,
            message: 'Error uploading files: ' + err.message,
            error: err.toString()
        });
    }
});


// --------------------------------------------------------------
// -- Route to download all files as a ZIP archive by tag
router.get('/downloadTag', async (req, res) => {
    const tag = req.query.tag;
    
    if (!tag) {
        return res.status(400).send('Tag parameter is required');
    }
    
    let filesCollection = db.collection("detconfigs");
    
    try {
        const files = await filesCollection.find({ tag }).toArray();

        files.forEach(file => {
            console.log("file: " + file.filename);           
            //            console.log("file: " + file.filename + " buffer size = " + file.content.buffer.length);           
        });
        console.log(" .. ..  ");

        if (files.length === 0) {
            return res.status(404).send('No files found for the given tag');
        }

        
        // -- Create a ZIP archive and stream it to the response
        const archive = archiver('zip', { zlib: { level: 9 } });
        res.setHeader('Content-Type', 'application/zip');
        res.setHeader('Content-Disposition', `attachment; filename=${tag}.zip`);
        
        // -- Pipe the archive to the response
        archive.pipe(res);
        
        // -- Append all files to the archive
        files.forEach(file => {
            const bufferStream = new stream.PassThrough();
            // -- note: .buffer is absolutely essential
            bufferStream.end(file.content.buffer);
            archive.append(bufferStream, { name: file.filename });
        });
        
        // -- Listen for any errors
        archive.on('error', (err) => {
            throw err;
        });
        
        // -- Finalize the archive (must be called to finish the stream)
        archive.finalize();
        
    } catch (err) {
        res.status(500).send('Error retrieving files: ' + err.message);
    }
});


// ----------------------------------------------------------------------
//moor>curl -X POST -F "tag=j1" -F "filename=j1/root.json" -F "file=@j1/root.json" http://localhost:5050/cdb/uploadJSON
router.post('/uploadJSON', singleUpload.single('file'), async (req, res) => {
         try {
        let collection = db.collection('detconfigs');
        
        const { tag, filename } = req.body; // Get 'tag' and 'filename' from form data
        const filePath = req.file.path; // Temporary path of the uploaded file

        // Read the file contents as a BLOB (binary data)
        const fileContent = fs.readFileSync(filePath);

        // Insert the document into MongoDB as a BLOB (Binary)
        await collection.insertOne({
            tag,
            filename,
            content: Binary(fileContent) // Store file content as Binary
        });

        // Clean up the temporary file
        fs.unlinkSync(filePath);

        res.status(200).send('File uploaded successfully');
    } catch (error) {
        console.error(error);
        res.status(500).send('Error uploading file');
    } 
});


// ----------------------------------------------------------------------
// moor>curl http://localhost:5050/cdb/downloadJSON/j2 -o root.json
router.get('/downloadJSON/:tag', async (req, res) => {
    try {
        let collection = db.collection("detconfigs");

        // Find the (last) document with the given tag
        const fileDocuments = await collection.find({ tag: req.params.tag }).toArray();
        if (fileDocuments.length > 0) {
            const fileDocument = fileDocuments[fileDocuments.length-1]
            if (fileDocument) {
                // The file content is stored as a BLOB (Binary), so convert it back to JSON
                const fileContentBuffer = fileDocument.content.buffer;
                const fileContentJson = JSON.parse(fileContentBuffer.toString('utf8'));

                // Send the parsed JSON content as the response
                res.json(fileContentJson);
            }
        } else {
            res.status(404).send('File not found');
        }
    } catch (error) {
        console.error(error);
        res.status(500).send('Error retrieving file');
    } 
});

// --------------------------------------------------------------
// -- Get tags for a specific globaltag
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
    let results = await tagsCollection.aggregate([
      { $match: { tag: { $in: globaltag.tags } } },
      { $group: { 
          _id: "$tag",
          tag: { $first: "$tag" },
          description: { $first: "$description" },
          iovs: { $first: "$iovs" }
        }
      },
      { $project: { 
          _id: 0,
          tag: 1,
          description: 1,
          iovs: 1
        }
      }
    ]).toArray();
    
    console.log("Found distinct tags:", results.length);
    console.log("First tag (if any):", results.length > 0 ? JSON.stringify(results[0]) : "none");
    
    res.send(results).status(200);
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
      return res.send([]).status(200);
    }
    
    console.log("Found tag with IOVs:", tag.iovs);
    
    // Then get all payloads for this tag
    let payloadsCollection = await db.collection("payloads");
    let query = {hash: { $regex: "^tag_" + req.params.tag + "_iov_" }};
    console.log("MongoDB query for payloads:", JSON.stringify(query));
    
    let results = await payloadsCollection.find(query).toArray();
    console.log("Found payloads:", results.length);
    
    // Add the IOVs array from the tag document to the response
    let response = {
      tag: tag.tag,
      iovs: tag.iovs,
      payloads: results
    };
    
    res.send(response).status(200);
  } catch (error) {
    console.error("Error in findPayloadsByTag:", error);
    res.status(500).send({ error: error.message });
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

// --------------------------------------------------------------
// -- Get summary of detconfigs tags and their counts
router.get("/findAll/detconfigsSummary", async (req, res) => {
  try {
    let detconfigsCollection = await db.collection("detconfigs");
    let results = await detconfigsCollection.aggregate([
      { $group: { _id: "$tag", count: { $sum: 1 } } },
      { $project: { _id: 0, tag: "$_id", count: 1 } },
      { $sort: { tag: 1 } }
    ]).toArray();
    res.send(results).status(200);
  } catch (error) {
    console.error("Error in detconfigsSummary:", error);
    res.status(500).send({ error: error.message });
  }
});

// --------------------------------------------------------------
// -- Delete all documents in detconfigs collection for a given tag
router.delete("/deleteTag", async (req, res) => {
  const tag = req.query.tag;
  
  if (!tag) {
    return res.status(400).json({ success: false, message: 'Tag parameter is required' });
  }
  
  try {
    let detconfigsCollection = await db.collection("detconfigs");
    const result = await detconfigsCollection.deleteMany({ tag });
    
    if (result.deletedCount === 0) {
      return res.status(404).json({ 
        success: false, 
        message: `No documents found with tag: ${tag}` 
      });
    }
    
    res.json({ 
      success: true, 
      message: `Successfully deleted ${result.deletedCount} documents for tag: ${tag}`,
      deletedCount: result.deletedCount
    });
  } catch (error) {
    console.error("Error deleting tag:", error);
    res.status(500).json({ 
      success: false, 
      message: 'Error deleting tag: ' + error.message 
    });
  }
});

// Serve cdb.html for GET /cdb
const __dirname = dirname(fileURLToPath(import.meta.url));

router.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "../public/cdb.html"));
});

export default router;
