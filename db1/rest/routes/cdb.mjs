import express from "express";
import {MongoClient, Binary} from "mongodb";

import multer from "multer";
import archiver from "archiver";
import stream from "stream";
import fs  from "fs";

const upload = multer(); // Multer for handling file uploads

// Multer setup to handle file uploads
const singleUpload = multer({ dest: 'uploads/' });

import db from "../db/conn.mjs";

const router = express.Router();

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



// -- Default CDB landing page 
router.get("/", async (req, res) => {
  console.log("serving default landing page");
  let collection = await db.collection("globaltags");
  let results = await collection.find({})
    .limit(50)
    .toArray();

  res.send(results).status(200);
});


// -- Get a single global tag
router.get("/findOne/globaltags/:id", async (req, res) => {
  console.log("serving /findOne/globaltags/" + req.params.id);
  let collection = await db.collection("globaltags");
  let query = {gt: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// -- Get a single tag/IOV
router.get("/findOne/tags/:id", async (req, res) => {
  console.log("serving /findOne/tags/" + req.params.id);
  let collection = await db.collection("tags");
  let query = {tag: req.params.id};
  let result = await collection.findOne(query);
  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// -- Get a single payload
router.get("/findOne/payloads/:id", async (req, res) => {
  console.log("serving /findOne/payloads/" + req.params.id);
  let collection = await db.collection("payloads");
  let query = {hash: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// -- Get a single configuration 
router.get("/findOne/configs/:id", async (req, res) => {
  console.log("serving /findOne/configs/" + req.params.id);
  let collection = await db.collection("configs");
  let query = {cfgHash: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// -- Get all globaltags
router.get("/findAll/globaltags", async (req, res) => {
  console.log("serving /findAll/globaltags/" + req.params.id);
  let collection = await db.collection("globaltags");
  let results = await collection.find({})
    .limit(50)
    .toArray();

  res.send(results).status(200);
});


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


// -- Upload multiple files to MongoDB
router.post('/uploadMany', upload.array('file'), async (req, res) => {
    if (!req.files || !req.body.tag) {
        return res.status(400).send('Files and tag are required');
    }
    
    let filesCollection = db.collection("detconfigs");
    
    try {
        const fileDocs = req.files.map(file => ({
            tag: req.body.tag,
            filename: file.originalname,
            content: file.buffer,
        }));
        
        const result = await filesCollection.insertMany(fileDocs);
        res.status(200).send(`Files uploaded successfully with IDs: ${result.insertedIds}`);
    } catch (err) {
        res.status(500).send('Error uploading files: ' + err.message);
    }
});


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

export default router;
