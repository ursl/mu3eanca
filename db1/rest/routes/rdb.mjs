import express from "express";
import db from "../db/conn.mjs";
//?? import { ObjectId } from "mongodb";

const router = express.Router();

// ----------------------------------------------------------------------
// -- Post a runrecord
// curl -X PUT -H "Content-Type: application/json" \
//   --data-binary @/Users/ursl/data/mu3e/json13/runrecords/runlog_007552.json \
//   http://localhost:5050/rdb/runrecords
router.put("/runrecords", async (req, res) => {
    console.log("PUT  /runrecords/ insert document from " + req.ip);
    const data    = req.body;
    let borData   = data.BOR;
    let runnumber = borData["Run number"];
    
    let collection = await db.collection("runrecords");
    
    let query = {"BOR.Run number": runnumber};
    let rDel = await collection.deleteMany(query);
    console.log("rDel ->" + JSON.stringify(rDel) + "<-");    
    console.log("runnumber ->" + runnumber + "<-");
    
    let newDocument = req.body;
    var currentdate = new Date(); 
    var datetime = currentdate.getFullYear() + "/" 
                 + (currentdate.getMonth()+1).toString().padStart(2, '0')  + "/" 
                 + currentdate.getDate().toString().padStart(2, '0') + " "
                 + currentdate.getHours().toString().padStart(2, '0') + ":"  
                 + currentdate.getMinutes().toString().padStart(2, '0') + ":" 
                 + currentdate.getSeconds().toString().padStart(2, '0');
    let addComment = {"date": datetime, "comment": "Database entry inserted "};
    if (newDocument.hasOwnProperty("History")) {
        newDocument["History"].push(addComment);
    } else {
        newDocument["History"] = [addComment];
    }

    let result = await collection.insertOne(newDocument);
    let retRes = 'CDB inserted:' + '\n' + JSON.stringify(req.body, null, 3) + '\n'
               + 'CDB result:' + '\n' + JSON.stringify(result, null, 3) + '\n';
    res.send(retRes).status(204);

});


// ----------------------------------------------------------------------
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


// ----------------------------------------------------------------------
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


// ----------------------------------------------------------------------
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


// ----------------------------------------------------------------------
// -- Adds a JSON fragment to the attributes array
router.put("/addAttribute/:id", async (req, res) => {
    let adata = req.body;

    // -- get first key (e.g. DQMOnline)
    let jsonKey;
    for(var key in adata) {
        jsonKey = key;
        break;
    }

    let runno = parseInt(req.params.id);

    let collection = await db.collection("runrecords");

    let query = {"BOR.Run number": runno};
    let result = await collection.findOne(query);
    let ndata = result;
    
    if (ndata.hasOwnProperty("Attributes")) {
        ndata["Attributes"].push(adata);
    } else {
        ndata["Attributes"] = [adata];
    }
    delete ndata["_id"];

    var currentdate = new Date(); 
    var datetime = currentdate.getFullYear() + "/" 
                 + (currentdate.getMonth()+1).toString().padStart(2, '0')  + "/" 
                 + currentdate.getDate().toString().padStart(2, '0') + " "
                 + currentdate.getHours().toString().padStart(2, '0') + ":"  
                 + currentdate.getMinutes().toString().padStart(2, '0') + ":" 
                 + currentdate.getSeconds().toString().padStart(2, '0');
    let addComment = {"date": datetime, "comment": "Added " + jsonKey};
    ndata.History.push(addComment);

    //    console.log("addAttribute> runno = " + runno ); 
    //    console.log("addAttribute> adata ->" + JSON.stringify(adata) + "<-"); 
    //    console.log("addAttribute> ndata ->" + JSON.stringify(ndata) + "<-"); 
    //    console.log("addAttribute> result ->" + JSON.stringify(result) + "<-"); 

    const nval = {$set: ndata};
    await collection.updateOne(query, nval, function(err, res) {
        if (err) throw err;
        console.log("1 document updated");
    });

    res.sendStatus(204);
});


export default router;
