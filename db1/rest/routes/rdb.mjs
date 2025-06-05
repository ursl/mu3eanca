import express from "express";
import db from "../db/conn.mjs";
import multer from "multer";
import path from "path";
import fs from "fs";
import { ObjectId } from "mongodb";

const router = express.Router();

// Configure multer for PDF uploads - store in memory
const upload = multer({
    storage: multer.memoryStorage(),
    fileFilter: function (req, file, cb) {
        if (file.mimetype !== 'application/pdf') {
            return cb(new Error('Only PDF files are allowed'));
        }
        cb(null, true);
    }
});

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

    var addComment = {"date": datetime, "comment": "Database entry inserted "};
    if (rDel.deletedCount > 0) {
        addComment = {"date": datetime, "comment": "Database entry updated "};
    }

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
    const runClass = req.query.runClass;
    const comment = req.query.comment;  // Add comment parameter

    console.log("Query params:", { nruns, minrun, maxrun, onlySignificant, starttime, stoptime, runClass, comment });

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

        // Add a field with the last RunInfo instance for filtering
        pipeline.push({
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
        });

        // If onlySignificant is "yes", filter by last RunInfo's Significant field
        if (onlySignificant === "yes") {
            pipeline.push({
                $match: {
                    "lastRunInfo.RunInfo.Significant": "true"
                }
            });
        }

        // If runClass is specified, filter by last RunInfo's Class field
        if (runClass) {
            pipeline.push({
                $match: {
                    "BOR.Run Class": runClass
                }
            });
        }

        // If comment is specified, filter by both RunInfo Comments and EOR Comments (case-insensitive)
        if (comment) {
            pipeline.push({
                $match: {
                    $or: [
                        { "lastRunInfo.RunInfo.Comments": { 
                            $regex: comment,
                            $options: 'i'  // 'i' makes the search case-insensitive
                        }},
                        { "EOR.Comments": { 
                            $regex: comment,
                            $options: 'i'  // 'i' makes the search case-insensitive
                        }}
                    ]
                }
            });
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
            'onlySignificant': onlySignificant || 'yes',  // Default to 'yes' if not specified
            'runClass': runClass || '',  // Pass the run class to the template
            'comment': comment || ''  // Pass the comment to the template
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

// ----------------------------------------------------------------------
// -- Add a PDF resource to a run record
// curl -X POST -F "pdf=@/path/to/file.pdf" http://localhost:5050/rdb/addResource/780
router.post("/addResource/:id", upload.single('pdf'), async (req, res) => {
    try {
        if (!req.file) {
            return res.status(400).send('No PDF file uploaded');
        }

        const runno = parseInt(req.params.id);
        const runrecordsCollection = await db.collection("runrecords");
        const uploadsCollection = await db.collection("uploads");
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result) {
            return res.status(404).send('Run not found');
        }

        // Create upload document
        const uploadDoc = {
            runNumber: runno,
            type: 'pdf',
            filename: req.file.originalname,
            uploadDate: new Date().toISOString(),
            description: req.body.description || 'PDF resource',
            content: req.file.buffer,
            contentType: req.file.mimetype
        };

        // Insert into uploads collection
        const uploadResult = await uploadsCollection.insertOne(uploadDoc);
        const uploadId = uploadResult.insertedId;

        // Create resource reference
        const resourceEntry = {
            type: 'pdf',
            uploadId: uploadId,
            filename: req.file.originalname,
            uploadDate: uploadDoc.uploadDate,
            description: uploadDoc.description
        };

        // Update runrecord document
        const updateData = { ...result };
        delete updateData._id;

        if (updateData.hasOwnProperty("Resources")) {
            updateData.Resources.push(resourceEntry);
        } else {
            updateData.Resources = [resourceEntry];
        }

        // Add to history
        const currentdate = new Date();
        const datetime = currentdate.getFullYear() + "/" 
                      + (currentdate.getMonth()+1).toString().padStart(2, '0') + "/" 
                      + currentdate.getDate().toString().padStart(2, '0') + " "
                      + currentdate.getHours().toString().padStart(2, '0') + ":"  
                      + currentdate.getMinutes().toString().padStart(2, '0') + ":" 
                      + currentdate.getSeconds().toString().padStart(2, '0');
        
        const addComment = {
            date: datetime,
            comment: `Added PDF resource: ${req.file.originalname}`
        };
        
        if (updateData.hasOwnProperty("History")) {
            updateData.History.push(addComment);
        } else {
            updateData.History = [addComment];
        }

        // Update runrecord
        const nval = { $set: updateData };
        await runrecordsCollection.updateOne(query, nval);
        
        res.status(200).json({
            message: 'PDF resource added successfully',
            resource: {
                type: resourceEntry.type,
                filename: resourceEntry.filename,
                uploadDate: resourceEntry.uploadDate,
                description: resourceEntry.description
            }
        });

    } catch (error) {
        console.error('Error adding PDF resource:', error);
        res.status(500).send('Error adding PDF resource: ' + error.message);
    }
});

// ----------------------------------------------------------------------
// -- Serve a PDF resource
// GET /rdb/resource/:runNumber/:index
router.get("/resource/:runNumber/:index", async (req, res) => {
    try {
        const runno = parseInt(req.params.runNumber);
        const index = parseInt(req.params.index);
        
        const runrecordsCollection = await db.collection("runrecords");
        const uploadsCollection = await db.collection("uploads");
        
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result || !result.Resources || !result.Resources[index]) {
            return res.status(404).send('Resource not found');
        }

        const resource = result.Resources[index];
        if (resource.type !== 'pdf' || !resource.uploadId) {
            return res.status(400).send('Not a valid PDF resource');
        }

        // Get the PDF from uploads collection
        const uploadDoc = await uploadsCollection.findOne({ _id: new ObjectId(resource.uploadId) });
        if (!uploadDoc) {
            return res.status(404).send('PDF file not found');
        }

        // Convert MongoDB Binary to Buffer
        const pdfBuffer = Buffer.from(uploadDoc.content.buffer);

        // Set appropriate headers for PDF
        res.writeHead(200, {
            'Content-Type': 'application/pdf',
            'Content-Length': pdfBuffer.length.toString(),
            'Content-Disposition': `inline; filename="${uploadDoc.filename}"`,
            'Cache-Control': 'public, max-age=31536000' // Cache for 1 year
        });
        
        // Send the PDF content
        res.end(pdfBuffer);

    } catch (error) {
        console.error('Error serving PDF resource:', error);
        res.status(500).send('Error serving PDF resource: ' + error.message);
    }
});

// ----------------------------------------------------------------------
// -- Delete a PDF resource
// DELETE /rdb/resource/:runNumber/:index
router.delete("/resource/:runNumber/:index", async (req, res) => {
    try {
        const runno = parseInt(req.params.runNumber);
        const index = parseInt(req.params.index);
        
        const runrecordsCollection = await db.collection("runrecords");
        const uploadsCollection = await db.collection("uploads");
        
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result || !result.Resources || !result.Resources[index]) {
            return res.status(404).send('Resource not found');
        }

        const resource = result.Resources[index];
        if (resource.type !== 'pdf' || !resource.uploadId) {
            return res.status(400).send('Not a valid PDF resource');
        }

        // Remove from uploads collection
        await uploadsCollection.deleteOne({ _id: new ObjectId(resource.uploadId) });

        // Update runrecord to remove the resource reference
        const updateData = { ...result };
        delete updateData._id;
        updateData.Resources.splice(index, 1);

        // Add to history
        const currentdate = new Date();
        const datetime = currentdate.getFullYear() + "/" 
                      + (currentdate.getMonth()+1).toString().padStart(2, '0') + "/" 
                      + currentdate.getDate().toString().padStart(2, '0') + " "
                      + currentdate.getHours().toString().padStart(2, '0') + ":"  
                      + currentdate.getMinutes().toString().padStart(2, '0') + ":" 
                      + currentdate.getSeconds().toString().padStart(2, '0');
        
        const addComment = {
            date: datetime,
            comment: `Removed PDF resource: ${resource.filename}`
        };
        
        if (updateData.hasOwnProperty("History")) {
            updateData.History.push(addComment);
        } else {
            updateData.History = [addComment];
        }

        // Update runrecord
        const nval = { $set: updateData };
        await runrecordsCollection.updateOne(query, nval);

        res.status(200).json({
            message: 'PDF resource deleted successfully'
        });

    } catch (error) {
        console.error('Error deleting PDF resource:', error);
        res.status(500).send('Error deleting PDF resource: ' + error.message);
    }
});

// ----------------------------------------------------------------------
// -- Get a single runrecord as JSON
router.get("/run/:id/json", async (req, res) => {
    let runno = parseInt(req.params.id);
    console.log("serving JSON for run " + req.params.id + " from " + req.ip);

    let collection = await db.collection("runrecords");

    let query = {"BOR.Run number": runno};
    let result = await collection.findOne(query);

    if (!result) {
        res.status(404).json({ error: "Run not found" });
    } else {
        res.json(result);
    }
});

export default router;
