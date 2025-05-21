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
    let eorData   = data.EOR;
    let runnumber = borData["Run number"];
    
    let collection = await db.collection("runrecords");
    
    let query = {"BOR.Run number": runnumber};
    let rDel = await collection.deleteMany(query);
    console.log("rDel ->" + JSON.stringify(rDel) + "<-");    
    console.log("runnumber ->" + runnumber + "<-");
    
    let newDocument = req.body;

    var docBORDate = borData["Start time"];
    var docEORDate = eorData["Stop time"];
    console.log("docBORDate ->" + docBORDate + "<-");
    console.log("docEORDate ->" + docEORDate + "<-");
    
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
// Helper function to build base query with significant run filtering
function buildBaseQuery(onlySignificant) {
    if (onlySignificant === "yes") {
        return {
            "Attributes": {
                $elemMatch: {
                    "RunInfo.Significant": "true"
                }
            }
        };
    }
    return {};
}


// ----------------------------------------------------------------------
// Helper function to merge query conditions
function mergeQueryConditions(baseQuery, additionalConditions) {
    return { ...baseQuery, ...additionalConditions };
}

// ----------------------------------------------------------------------
router.get("/", async (req, res) => {
    let collection = await db.collection("runrecords");
    let MAXRUNS = 1000;

    console.log("----------------------------------------------------");
    console.log("req.query: " + JSON.stringify(req.query));
    console.log("serving from RDB / " + req.params.id);

    // Parse query parameters with onlySignificant defaulting to "yes"
    const nruns = req.query.nRun ? Number(req.query.nRun) : -1;
    const minrun = Number(req.query.minRun) || -1;
    const maxrun = Number(req.query.maxRun) || -1;
    const onlySignificant = req.query.onlySignificant === "no" ? "no" : "yes";  // Default to "yes" unless explicitly set to "no"
    const starttime = req.query.startTime;
    const stoptime = req.query.stopTime;

    console.log("Query params:", { nruns, minrun, maxrun, onlySignificant, starttime, stoptime });

    try {
        // Build the aggregation pipeline
        let pipeline = [];
        
        // First stage: Filter by run number range if specified
        if (minrun > -1 && maxrun > 0) {
            pipeline.push({ $match: { "BOR.Run number": { $gte: minrun, $lte: maxrun } } });
        } else if (minrun > -1) {
            pipeline.push({ $match: { "BOR.Run number": { $gte: minrun } } });
        } else if (maxrun > -1) {
            pipeline.push({ $match: { "BOR.Run number": { $lte: maxrun } } });
        }

        // Filter by time if specified
        if (starttime && stoptime) {
            pipeline.push({ 
                $match: { 
                    "BOR.Start time": { $regex: starttime },
                    "BOR.Stop time": { $regex: stoptime }
                } 
            });
        } else if (starttime) {
            pipeline.push({ $match: { "BOR.Start time": { $regex: starttime } } });
        } else if (stoptime) {
            pipeline.push({ $match: { "BOR.Stop time": { $regex: stoptime } } });
        }

        // If onlySignificant is "yes", add stages to filter by last RunInfo
        if (onlySignificant === "yes") {
            pipeline.push(
                // Add a field with the last RunInfo instance
                {
                    $addFields: {
                        lastRunInfo: {
                            $arrayElemAt: [
                                {
                                    $filter: {
                                        input: "$Attributes",
                                        as: "attr",
                                        cond: { $eq: [{ $type: "$$attr.RunInfo" }, "object"] }
                                    }
                                },
                                -1  // Get the last element
                            ]
                        }
                    }
                },
                // Match only documents where the last RunInfo is significant
                {
                    $match: {
                        "lastRunInfo.RunInfo.Significant": "true"
                    }
                }
            );
        }

        // Sort by run number descending
        pipeline.push({ $sort: { "BOR.Run number": -1 } });

        // Apply limit if specified
        if (nruns > 0) {
            pipeline.push({ $limit: nruns });
        } else {
            pipeline.push({ $limit: MAXRUNS });
        }

        console.log("Aggregation pipeline:", JSON.stringify(pipeline, null, 2));
        
        const result = await collection.aggregate(pipeline).toArray();
        console.log("Query successful, found", result.length, "results");
        
        res.render('index', {
            'data': result,
            'onlySignificant': onlySignificant || 'yes'  // Default to 'yes' if not specified
        });
    } catch (error) {
        console.error("Error executing query:", error);
        console.error("Error stack:", error.stack);
        res.status(500).send("Internal server error: " + error.message);
    }
});


// ----------------------------------------------------------------------
// -- Get a single runrecord
router.get("/run/:id", async (req, res) => {
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
// -- Get all run numbers
router.get("/allRunNumbers", async (req, res) => {
    console.log("serving from RDB /allRunNumbers " + req.params.id);
    let collection = await db.collection("runrecords");
    let results = await collection.find({})
      .toArray();

    let runNumbers = results.map(record => record.BOR["Run number"]);
    res.send(runNumbers).status(200);
  
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
// curl -X PUT -H "Content-Type: application/json" \
//      --data-binary @../../db1/rest/dqTemplate.json \
//      pc11740:5050/rdb/addAttribute/780
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
