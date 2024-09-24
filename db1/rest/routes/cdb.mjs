import express from "express";

import multer from "multer";
import archiver from "archiver";
import stream from "stream";

const upload = multer(); // Multer for handling file uploads

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


// FIXME still used? not all in rdb?
// Post a runrecord
router.put("/runrecords", async (req, res) => {
  console.log("PUT  /runrecords/ insert document from " + req.ip);
  const data    = req.body;
  let borData   = data.BOR;
  let runnumber = borData["Run number"];

  let collection = await db.collection("runrecords");
  
  let query = {"BOR.Run number": runnumber};
  let rDel = await collection.deleteMany(query);
  console.log("rDel ->" + rDel + "<-");

  console.log("runnumber ->" + runnumber + "<-");

  let newDocument = req.body;
  let result = await collection.insertOne(newDocument);
  let retRes = 'CDB inserted:' + '\n' + JSON.stringify(req.body, null, 3) + '\n'
      + 'CDB result:' + '\n' + JSON.stringify(result, null, 3) + '\n';
  res.send(retRes).status(204);

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
            console.log("file: " + file.filename + " buffer size = " + file.content.buffer.length);           
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


export default router;
