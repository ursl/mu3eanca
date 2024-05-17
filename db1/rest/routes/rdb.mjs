import express from "express";
import db from "../db/conn.mjs";
import { ObjectId } from "mongodb";

const router = express.Router();

// -- index page (with possible filters)
router.get("/", async (req, res) => {
    let collection = await db.collection("runrecords");

    let nruns = -1;
    nruns = Number(req.query.nRun);
    console.log("nruns = " + nruns);

    let minrun = -1;
    minrun = Number(req.query.minRun);
    console.log("minrun = " + minrun);
    let maxrun = -1;
    maxrun = Number(req.query.maxRun);
    console.log("maxrun = " + maxrun);


    const options = {
        // Sort returned documents in ascending order by title (A->Z)
        sort: { "BOR.Run number": -1 }
    };
    let query = { };
    if (nruns > 0) {
        const result = await collection.find(query, options).limit(nruns).toArray();
        res.render('index', {'data': result});
        return;
    } else if (minrun > -1) {
        if (maxrun > -1) {
            query = {"BOR.Run number": {$gte: minrun, $lte: maxrun}};
            console.log("query: " + JSON.stringify(query));
            const result = await collection.find(query).toArray();
            console.log("result: " + JSON.stringify(result));
            res.render('index', {'data': result});
            return;
        } else {
            query = {"BOR.Run number": {$gte: minrun} };
            console.log("query: " + JSON.stringify(query));
            const result = await collection.find(query).toArray();
            console.log("result: " + JSON.stringify(result));
            res.render('index', {'data': result});
            return;
        }
    }
    const result = await collection.find(query, options).limit(15).toArray();
    console.log("default result: " + JSON.stringify(result));
    res.render('index', {'data': result});

});

// -- Get a single runrecord
router.get("/:id", async (req, res) => {
    let runno = parseInt(req.params.id);
    console.log("serving ... /rdb/" + req.params.id + " from " + req.ip);

    let collection = await db.collection("runrecords");

    let query = {"BOR.Run number": runno};
    let result = await collection.findOne(query);

    if (!result) res.send("Not found").status(404);
    else {
        console.log("calling singleRun with result: " + JSON.stringify(result));
        res.render('singleRun', {'data': result, 'runs': req.query.runs});
    }
});

export default router;
