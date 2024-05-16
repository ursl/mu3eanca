import express from "express";
import db from "../db/conn.mjs";
import { ObjectId } from "mongodb";

const router = express.Router();

// -- Get a single runrecord
router.get("/:id", async (req, res) => {
    let runno = parseInt(req.params.id);
    console.log("serving ... /rdb/" + req.params.id + " from " + req.ip);

    let collection = await db.collection("runrecords");

    let query = {"BOR.Run number": runno};
    let result = await collection.findOne(query);

    if (!result) res.send("Not found").status(404);
    else {
        const data = {
            name: "jaimin",
            email: "jaimin@gmail.com"
        };

        console.log("calling render");
        res.render('index', data);
    }
});

export default router;
