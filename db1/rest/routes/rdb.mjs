import express from "express";
import db from "../db/conn.mjs";
import { ObjectId } from "mongodb";

const router = express.Router();

// -- Get a single runrecord
router.get("/:id", async (req, res) => {
  let runno = parseInt(req.params.id);
  console.log("serving ... /findOne/runrecords/" + req.params.id + " from " + req.ip);
  // console.log("req.params.id ->" + req.params.id + "<-" );
  // console.log(typeof req.params.id); // string
  // console.log("runno = " + runno);

  let collection = await db.collection("runrecords");

  let query = {"BOR.Run number": runno};
  let result = await collection.findOne(query);

  if (!result) res.send("Not found").status(404);
  else res.send(result).status(200);
});



export default router;
