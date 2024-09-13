import express from "express";
import db from "../db/conn.mjs";
//?? import { ObjectId } from "mongodb";

const router = express.Router();

// -- index page (with possible filters)
router.get("/", async (req, res) => {
    let collection = await db.collection("runrecords");

    // -- number of runs to show
    let nruns = -1;
    nruns = Number(req.query.nRun);
    console.log("nruns = " + nruns);

    // -- run range to show
    let minrun = -1;
    minrun = Number(req.query.minRun);
    console.log("minrun = " + minrun);
    let maxrun = -1;
    maxrun = Number(req.query.maxRun);
    console.log("maxrun = " + maxrun);

    // -- time filtering attempts
    let starttime = "unset";
    starttime = req.query.startTime;
    console.log("starttime = " + starttime);
    let stoptime = "unset";
    stoptime = req.query.stopTime;
    console.log("stoptime = " + stoptime);

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
        if (maxrun > 0) {
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
    } else if (maxrun > -1) {
        query = {"BOR.Run number": {$lte: maxrun}};
        console.log("query: " + JSON.stringify(query));
        const result = await collection.find(query).toArray();
        console.log("result: " + JSON.stringify(result));
        res.render('index', {'data': result});
        return;
    } else if (starttime !== undefined) {
        query = {"BOR.Start time": {$regex: starttime}};
        console.log("query: " + JSON.stringify(query));
        const result = await collection.find(query).toArray();
        console.log("result: " + JSON.stringify(result));
        res.render('index', {'data': result});
        return;
    } else if (stoptime !== undefined) {
        query = {"BOR.Stop time": {$regex: stoptime}};
        console.log("query: " + JSON.stringify(query));
        const result = await collection.find(query).toArray();
        console.log("result: " + JSON.stringify(result));
        res.render('index', {'data': result});
        return;
    } else if ((starttime !== undefined) && (stoptime !== undefined)) {
        query = [{"BOR.Start time": {$regex: starttime}}, {"BOR.Stop time": {$regex: stoptime}}];
        console.log("query: " + JSON.stringify(query));
        const result = await collection.find(query).toArray();
        console.log("result: " + JSON.stringify(result));
        res.render('index', {'data': result});
        return;
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

// -- Post a single runrecord
router.post("/updateRun", async (req, res) => {
    console.log("=>router.post, req:" + JSON.stringify(req.body));

    var bor = req.body.BOR;    
    const runNumber = bor["Run number"];

    let collection = await db.collection("runrecords");

    let runno = parseInt(runNumber);
    let query = {"BOR.Run number": runno};

    let ndata = req.body;
    delete ndata["_id"];
    console.log("ndata: " + JSON.stringify(ndata));                

    const nval = {$set: ndata};
    await collection.updateOne(query, nval, function(err, res) {
        if (err) throw err;
        console.log("1 document updated");
    });
    // await db.save(); // FIXME: will lead to "500 (Internal Server Error)"

});


export default router;
