import express from "express";
import db from "../db/conn.mjs";

const router = express.Router();

// ----------------------------------------------------------------------
// -- index page (with possible filters)
router.get("/", async (req, res) => {
    let collection = await db.collection("detconfigs");

    let query = { };
    
    const result = await collection.find(query).project({ filename: 1, tag: 1}).toArray();
    console.log("default result: " + JSON.stringify(result));
    res.render('detconfigs', {'data': result});

});

export default router;
