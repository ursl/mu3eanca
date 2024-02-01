import express from "express";
import db from "../db/conn.mjs";
import { ObjectId } from "mongodb";

const router = express.Router();

// Get a single runrecord
router.get("/findOne/runrecords/:id", async (req, res) => {
  let runno = parseInt(req.params.id);
  console.log("serving 14 /findOne/runrecords/" + req.params.id);
  // console.log("req.params.id ->" + req.params.id + "<-" );
  // console.log(typeof req.params.id); // string
  // console.log("runno = " + runno);

  let collection = await db.collection("runrecords");
//OK   let query = {"BOR.RunNumber": 12};
//NOK  let query = {"BOR.RunNumber": req.params.id};
//OK   let query = {"BOR.RunNumber": runno};

  let query = {"BOR.Run Number": runno};
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


export default router;
