import express from "express";

import multer from "multer";

const upload = multer(); // Multer for handling file uploads

import db from "../db/conn.mjs";
//import { ObjectId } from "mongodb";

const router = express.Router();

// Get a single runrecord
router.get("/findOne/runrecords/:id", async (req, res) => {
  let runno = parseInt(req.params.id);
  console.log("serving /findOne/runrecords/" + req.params.id + " from " + req.ip);
  // console.log("req.params.id ->" + req.params.id + "<-" );
  // console.log(typeof req.params.id); // string
  // console.log("runno = " + runno);

  let collection = await db.collection("runrecords");
//OK   let query = {"BOR.RunNumber": 12};
//NOK  let query = {"BOR.RunNumber": req.params.id};
//OK   let query = {"BOR.RunNumber": runno};

  let query = {"BOR.Run number": runno};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});

// Get a single global tag
router.get("/findOne/globaltags/:id", async (req, res) => {
  console.log("serving /findOne/globaltags/" + req.params.id);
  let collection = await db.collection("globaltags");
  let query = {gt: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});

// Get a single tag/IOV
router.get("/findOne/tags/:id", async (req, res) => {
  console.log("serving /findOne/tags/" + req.params.id);
  let collection = await db.collection("tags");
  let query = {tag: req.params.id};
  let result = await collection.findOne(query);
  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});

// Get a single payload
router.get("/findOne/payloads/:id", async (req, res) => {
  console.log("serving /findOne/payloads/" + req.params.id);
  let collection = await db.collection("payloads");
  let query = {hash: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// Get a single configuration 
router.get("/findOne/configs/:id", async (req, res) => {
  console.log("serving /findOne/configs/" + req.params.id);
  let collection = await db.collection("configs");
  let query = {cfgHash: req.params.id};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});


// Get all globaltags
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


// Upload a file to MongoDB
router.post('/upload', upload.single('file'), async (req, res) => {
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


export default router;
