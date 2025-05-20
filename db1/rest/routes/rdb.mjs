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
router.get("/", async (req, res) => {
    let collection = await db.collection("runrecords");
    let MAXRUNS = 1000;

    console.log("----------------------------------------------------");
    console.log("req.query: " + JSON.stringify(req.query));
    console.log("serving from RDB / " + req.params.id);
    // -- number of runs to show
    let nruns = -1;
    if (req.query.nRun) {
        nruns = Number(req.query.nRun);
    } 

    console.log("nruns = " + nruns);

    // -- run range to show
    let minrun = -1;
    minrun = Number(req.query.minRun);
    console.log("minrun = " + minrun);
    let maxrun = -1;
    maxrun = Number(req.query.maxRun);
    console.log("maxrun = " + maxrun);

    // -- significant runs filtering
    let onlySignificant = "unset";
    onlySignificant = req.query.onlySignificant;
    console.log("HalloHallo>  onlySignificant = " + onlySignificant);
    let querySignificant  = [{
        $expr: {
            $eq: [
                {$getField: {
                    field: "Significant",
                    input: {$arrayElemAt: [
                        {$filter: {
                            input: "$Attributes",
                            as: "attr",
                            cond: {$eq: [{$getField: {field: "$$ROOT", input: "$$attr"}}, "RunInfo"]}
                        }},
                        -1
                    ]}
                }},
                "true"
            ]
        }
    }];


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
        if (onlySignificant !== undefined) {
            if (onlySignificant === "yes") {    
                query = {
                    "Attributes": {
                        $elemMatch: {
                            "RunInfo.Significant": "true"
                        }
                    }
                };
            } else {
                query = {};
            }
        }         
        try {
            console.log("Query with nruns:", JSON.stringify(query, null, 2));
            const result = await collection.find(query, options).limit(nruns).toArray();
            console.log("Query successful, found", result.length, "results");
            res.render('index', {'data': result});
        } catch (error) {
            console.error("Error in nruns query:", error);
            console.error("Error stack:", error.stack);
            res.status(500).send("Internal server error: " + error.message);
        }
        return;
    } else if (minrun > -1) {
        if (maxrun > 0) {
            if (onlySignificant === "yes") {    
                query = {
                    "Attributes": {
                        $elemMatch: {
                            "RunInfo.Significant": "true"
                        }
                    },
                    "BOR.Run number": {$gte: minrun, $lte: maxrun}
                };
            } else {
                query = {"BOR.Run number": {$gte: minrun, $lte: maxrun}};
            }
            try {
                console.log("Query with min/max run:", JSON.stringify(query, null, 2));
                const result = await collection.find(query).toArray();
                console.log("Query successful, found", result.length, "results");
                res.render('index', {'data': result});
            } catch (error) {
                console.error("Error in min/max run query:", error);
                console.error("Error stack:", error.stack);
                res.status(500).send("Internal server error: " + error.message);
            }
            return;
        } else {
            if (onlySignificant === "yes") {    
                query = {
                    "Attributes": {
                        $elemMatch: {
                            "RunInfo.Significant": "true"
                        }
                    },
                    "BOR.Run number": {$gte: minrun}
                };
            } else {
                query = {"BOR.Run number": {$gte: minrun}};
            }
            try {
                console.log("Query with min run:", JSON.stringify(query, null, 2));
                const result = await collection.find(query).toArray();
                console.log("Query successful, found", result.length, "results");
                res.render('index', {'data': result});
            } catch (error) {
                console.error("Error in min run query:", error);
                console.error("Error stack:", error.stack);
                res.status(500).send("Internal server error: " + error.message);
            }
            return;
        }
    } else if (maxrun > -1) {
        if (onlySignificant === "yes") {    
            query = {
                "Attributes": {
                    $elemMatch: {
                        "RunInfo.Significant": "true"
                    }
                },
                "BOR.Run number": {$lte: maxrun}
            };
        } else {
            query = {"BOR.Run number": {$lte: maxrun}};
        }
        try {
            console.log("Query with max run:", JSON.stringify(query, null, 2));
            const result = await collection.find(query).toArray();
            console.log("Query successful, found", result.length, "results");
            res.render('index', {'data': result});
        } catch (error) {
            console.error("Error in max run query:", error);
            console.error("Error stack:", error.stack);
            res.status(500).send("Internal server error: " + error.message);
        }
        return;
    } else if (starttime !== undefined) {
        if (onlySignificant === "yes") {    
            query = {
                "Attributes": {
                    $elemMatch: {
                        "RunInfo.Significant": "true"
                    }
                },
                "BOR.Start time": {$regex: starttime}
            };
        } else {
            query = {"BOR.Start time": {$regex: starttime}};
        }
        try {
            console.log("Query with start time:", JSON.stringify(query, null, 2));
            const result = await collection.find(query).toArray();
            console.log("Query successful, found", result.length, "results");
            res.render('index', {'data': result});
        } catch (error) {
            console.error("Error in start time query:", error);
            console.error("Error stack:", error.stack);
            res.status(500).send("Internal server error: " + error.message);
        }
        return;
    } else if (stoptime !== undefined) {
        if (onlySignificant === "yes") {    
            query = {
                "Attributes": {
                    $elemMatch: {
                        "RunInfo.Significant": "true"
                    }
                },
                "BOR.Stop time": {$regex: stoptime}
            };
        } else {
            query = {"BOR.Stop time": {$regex: stoptime}};
        }
        try {
            console.log("Query with stop time:", JSON.stringify(query, null, 2));
            const result = await collection.find(query).toArray();
            console.log("Query successful, found", result.length, "results");
            res.render('index', {'data': result});
        } catch (error) {
            console.error("Error in stop time query:", error);
            console.error("Error stack:", error.stack);
            res.status(500).send("Internal server error: " + error.message);
        }
        return;
    } else if ((starttime !== undefined) && (stoptime !== undefined)) {
        if (onlySignificant === "yes") {    
            query = {
                "Attributes": {
                    $elemMatch: {
                        "RunInfo.Significant": "true"
                    }
                },
                "BOR.Start time": {$regex: starttime},
                "BOR.Stop time": {$regex: stoptime}
            };
        } else {
            query = {
                "BOR.Start time": {$regex: starttime},
                "BOR.Stop time": {$regex: stoptime}
            };
        }
        try {
            console.log("Query with start/stop time:", JSON.stringify(query, null, 2));
            const result = await collection.find(query).toArray();
            console.log("Query successful, found", result.length, "results");
            res.render('index', {'data': result});
        } catch (error) {
            console.error("Error in start/stop time query:", error);
            console.error("Error stack:", error.stack);
            res.status(500).send("Internal server error: " + error.message);
        }
        return;
    } else if (onlySignificant !== undefined) {
        if (onlySignificant === "yes") {
            query = {
                "Attributes": {
                    $elemMatch: {
                        "RunInfo.Significant": "true"
                    }
                }
            };
        } else {
            query = {};
        }
        try {
            console.log("Query with only significant:", JSON.stringify(query, null, 2));
            const result = await collection.find(query).toArray();
            console.log("Query successful, found", result.length, "results");
            res.render('index', {'data': result, 'onlySignificant': onlySignificant});
        } catch (error) {
            console.error("Error in only significant query:", error);
            console.error("Error stack:", error.stack);
            res.status(500).send("Internal server error: " + error.message);
        }
        return;
    }

    // Default query - show significant runs by default
    console.log("Default query - show significant runs by default");
    try {
        query = {
            "Attributes": {
                $elemMatch: {
                    "RunInfo.Significant": "true"
                }
            }
        };
        console.log("Query constructed:", JSON.stringify(query, null, 2));
        const result = await collection.find(query, options).limit(MAXRUNS).toArray();
        console.log("Query successful, found", result.length, "results");
        res.render('index', {'data': result, 'onlySignificant': 'yes'});
    } catch (error) {
        console.error("Error in default query:", error);
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
