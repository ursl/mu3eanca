import express from "express";
import db from "../db/conn.mjs";
import multer from "multer";
import path from "path";
import fs from "fs";
import { ObjectId } from "mongodb";
import { GridFSBucket } from "mongodb";

const router = express.Router();

// Initialize GridFS bucket
const bucket = new GridFSBucket(db, {
    bucketName: 'uploads'
});

// Configure multer for file uploads - store in memory
const upload = multer({
    storage: multer.memoryStorage(),
    limits: {
        fileSize: 100 * 1024 * 1024  // 100MB limit
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
    console.log("serving from RDB /allRunNumbers");
    let collection = await db.collection("runrecords");
    let results = await collection.find({})
      .sort({ "BOR.Run number": -1 })  // Sort by run number in descending order
      .toArray();

    let runNumbers = results.map(record => record.BOR["Run number"]);
    res.send(runNumbers);
});

// ----------------------------------------------------------------------
// -- Get last run number
router.get("/lastRunNumber", async (req, res) => {
    try {
        let collection = await db.collection("runrecords");
        let result = await collection.find({})
          .sort({ "BOR.Run number": -1 })
          .limit(1)
          .toArray();

        if (result && result.length > 0) {
            return res.json(result[0].BOR["Run number"]);
        }
        return res.sendStatus(404);
    } catch (error) {
        return res.sendStatus(500);
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
// -- Add a resource to a run record
// curl -X POST -F "pdf=@/path/to/file" http://localhost:5050/rdb/addResource/780
router.post("/addResource/:id", upload.single('pdf'), async (req, res) => {
    try {
        if (!req.file) {
            return res.status(400).send('No file uploaded');
        }

        const runno = parseInt(req.params.id);
        const runrecordsCollection = await db.collection("runrecords");
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result) {
            return res.status(404).send('Run not found');
        }

        // Upload to GridFS
        const uploadStream = bucket.openUploadStream(req.file.originalname, {
            metadata: {
                runNumber: runno,
                type: req.file.mimetype,
                description: req.body.description || 'Resource',
                contentType: req.file.mimetype
            }
        });

        // Write the file buffer to GridFS
        uploadStream.write(req.file.buffer);
        uploadStream.end();

        // Wait for the upload to complete
        const uploadResult = await new Promise((resolve, reject) => {
            uploadStream.on('finish', resolve);
            uploadStream.on('error', reject);
        });

        // Create resource reference
        const resourceEntry = {
            type: req.file.mimetype,
            fileId: uploadStream.id,
            filename: req.file.originalname,
            uploadDate: new Date().toISOString(),
            description: req.body.description || 'Resource'
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
            comment: `Added resource: ${req.file.originalname} (${req.file.mimetype})`
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
            message: 'Resource added successfully',
            resource: {
                type: resourceEntry.type,
                filename: resourceEntry.filename,
                uploadDate: resourceEntry.uploadDate,
                description: resourceEntry.description
            }
        });

    } catch (error) {
        console.error('Error adding resource:', error);
        res.status(500).send('Error adding resource: ' + error.message);
    }
});

// ----------------------------------------------------------------------
// -- Serve a resource
// GET /rdb/resource/:runNumber/:index
router.get("/resource/:runNumber/:index", async (req, res) => {
    try {
        const runno = parseInt(req.params.runNumber);
        const index = parseInt(req.params.index);
        
        const runrecordsCollection = await db.collection("runrecords");
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result || !result.Resources || !result.Resources[index]) {
            return res.status(404).send('Resource not found');
        }

        const resource = result.Resources[index];
        if (!resource.fileId) {
            return res.status(400).send('Not a valid resource');
        }

        // Get file metadata from GridFS
        const files = await bucket.find({ _id: new ObjectId(resource.fileId) }).toArray();
        if (files.length === 0) {
            return res.status(404).send('File not found');
        }

        const file = files[0];

        // Set appropriate headers
        res.writeHead(200, {
            'Content-Type': file.metadata.contentType || 'application/octet-stream',
            'Content-Length': file.length,
            'Content-Disposition': `inline; filename="${file.filename}"`,
            'Cache-Control': 'public, max-age=31536000' // Cache for 1 year
        });

        // Stream the file from GridFS to the response
        const downloadStream = bucket.openDownloadStream(new ObjectId(resource.fileId));
        downloadStream.pipe(res);

    } catch (error) {
        console.error('Error serving resource:', error);
        res.status(500).send('Error serving resource: ' + error.message);
    }
});

// ----------------------------------------------------------------------
// -- Delete a resource
// DELETE /rdb/resource/:runNumber/:index
router.delete("/resource/:runNumber/:index", async (req, res) => {
    try {
        const runno = parseInt(req.params.runNumber);
        const index = parseInt(req.params.index);
        
        const runrecordsCollection = await db.collection("runrecords");
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result || !result.Resources || !result.Resources[index]) {
            return res.status(404).send('Resource not found');
        }

        const resource = result.Resources[index];
        if (!resource.fileId) {
            return res.status(400).send('Not a valid resource');
        }

        // Delete from GridFS
        await bucket.delete(new ObjectId(resource.fileId));

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
            comment: `Removed resource: ${resource.filename} (${resource.type})`
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
            message: 'Resource deleted successfully'
        });

    } catch (error) {
        console.error('Error deleting resource:', error);
        res.status(500).send('Error deleting resource: ' + error.message);
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

// ----------------------------------------------------------------------
// -- Remove resources with specific description
// curl -X PUT -H "Content-Type: application/json" \
//      --data '{"description": "vtx"}' \
//      http://localhost:5050/rdb/removeResources/780
router.put("/removeResources/:id", async (req, res) => {
    try {
        const runno = parseInt(req.params.id);
        const description = req.body.description;

        if (!description) {
            return res.status(400).send('Description is required');
        }

        const runrecordsCollection = await db.collection("runrecords");
        const query = {"BOR.Run number": runno};
        const result = await runrecordsCollection.findOne(query);

        if (!result) {
            return res.status(404).send('Run not found');
        }

        // Get resources to be deleted for GridFS cleanup
        const resourcesToDelete = result.Resources?.filter(r => r.description === description) || [];
        
        // Delete files from GridFS
        for (const resource of resourcesToDelete) {
            if (resource.type === 'pdf' && resource.fileId) {
                await bucket.delete(new ObjectId(resource.fileId));
            }
        }

        // Update runrecord to remove resources with matching description
        const updateData = { ...result };
        delete updateData._id;
        
        if (updateData.Resources) {
            updateData.Resources = updateData.Resources.filter(r => r.description !== description);
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
            comment: `Removed resources with description: ${description}`
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
            message: 'Resources removed successfully',
            removedCount: resourcesToDelete.length
        });

    } catch (error) {
        console.error('Error removing resources:', error);
        res.status(500).send('Error removing resources: ' + error.message);
    }
});

// ----------------------------------------------------------------------
// -- Bulk update significant status for multiple runs
router.post("/bulk/significant", async (req, res) => {
    try {
        const { runs, value } = req.body;
        if (!Array.isArray(runs) || typeof value !== 'boolean') {
            return res.status(400).json({ error: 'Invalid request parameters' });
        }

        const collection = await db.collection("runrecords");
        let updated = 0;

        for (const runNumber of runs) {
            const query = { "BOR.Run number": parseInt(runNumber) };
            const run = await collection.findOne(query);
            
            if (run) {
                // Get current date for history
                const currentdate = new Date();
                const datetime = currentdate.getFullYear() + "/" 
                    + (currentdate.getMonth()+1).toString().padStart(2, '0') + "/" 
                    + currentdate.getDate().toString().padStart(2, '0') + " "
                    + currentdate.getHours().toString().padStart(2, '0') + ":"  
                    + currentdate.getMinutes().toString().padStart(2, '0') + ":" 
                    + currentdate.getSeconds().toString().padStart(2, '0');

                // Get the last RunInfo record or create a new one with default values
                const lastRunInfo = run.Attributes && run.Attributes.length > 0 
                    ? run.Attributes[run.Attributes.length - 1].RunInfo || {}
                    : {};

                // Create new RunInfo preserving all fields
                const runInfo = {
                    date: datetime,
                    Significant: value.toString(),
                    Comments: lastRunInfo.Comments || "unset",
                    Class: lastRunInfo.Class || "unset",
                    Quality: lastRunInfo.Quality || "unset",
                    Status: lastRunInfo.Status || "unset",
                    Type: lastRunInfo.Type || "unset",
                    Beam: lastRunInfo.Beam || "unset",
                    Target: lastRunInfo.Target || "unset",
                    Trigger: lastRunInfo.Trigger || "unset",
                    DAQ: lastRunInfo.DAQ || "unset",
                    Detector: lastRunInfo.Detector || "unset",
                    Environment: lastRunInfo.Environment || "unset",
                    Other: lastRunInfo.Other || "unset"
                };

                // Update document
                const update = {
                    $push: {
                        Attributes: {
                            RunInfo: runInfo
                        }
                    }
                };

                const result = await collection.updateOne(query, update);
                if (result.modifiedCount > 0) {
                    updated++;
                }
            }
        }

        res.json({ updated });
    } catch (error) {
        console.error('Error in bulk significant update:', error);
        res.status(500).json({ error: error.message });
    }
});

// ----------------------------------------------------------------------
// -- Bulk update comments for multiple runs
router.post("/bulk/comments", async (req, res) => {
    try {
        const { runs, comments } = req.body;
        if (!Array.isArray(runs) || typeof comments !== 'string') {
            return res.status(400).json({ error: 'Invalid request parameters' });
        }

        const collection = await db.collection("runrecords");
        let updated = 0;

        for (const runNumber of runs) {
            const query = { "BOR.Run number": parseInt(runNumber) };
            const run = await collection.findOne(query);
            
            if (run) {
                // Get current date for history
                const currentdate = new Date();
                const datetime = currentdate.getFullYear() + "/" 
                    + (currentdate.getMonth()+1).toString().padStart(2, '0') + "/" 
                    + currentdate.getDate().toString().padStart(2, '0') + " "
                    + currentdate.getHours().toString().padStart(2, '0') + ":"  
                    + currentdate.getMinutes().toString().padStart(2, '0') + ":" 
                    + currentdate.getSeconds().toString().padStart(2, '0');

                // Get the last RunInfo record or create a new one with default values
                const lastRunInfo = run.Attributes && run.Attributes.length > 0 
                    ? run.Attributes[run.Attributes.length - 1].RunInfo || {}
                    : {};

                // Create new RunInfo preserving all fields
                const runInfo = {
                    date: datetime,
                    Significant: lastRunInfo.Significant || "unset",
                    Comments: comments,
                    Class: lastRunInfo.Class || "unset",
                    Quality: lastRunInfo.Quality || "unset",
                    Status: lastRunInfo.Status || "unset",
                    Type: lastRunInfo.Type || "unset",
                    Beam: lastRunInfo.Beam || "unset",
                    Target: lastRunInfo.Target || "unset",
                    Trigger: lastRunInfo.Trigger || "unset",
                    DAQ: lastRunInfo.DAQ || "unset",
                    Detector: lastRunInfo.Detector || "unset",
                    Environment: lastRunInfo.Environment || "unset",
                    Other: lastRunInfo.Other || "unset"
                };

                // Update document
                const update = {
                    $push: {
                        Attributes: {
                            RunInfo: runInfo
                        }
                    }
                };

                const result = await collection.updateOne(query, update);
                if (result.modifiedCount > 0) {
                    updated++;
                }
            }
        }

        res.json({ updated });
    } catch (error) {
        console.error('Error in bulk comments update:', error);
        res.status(500).json({ error: error.message });
    }
});

// ----------------------------------------------------------------------
// -- Bulk update class for multiple runs
router.post("/bulk/class", async (req, res) => {
    try {
        const { runs, class: runClass } = req.body;
        if (!Array.isArray(runs) || typeof runClass !== 'string') {
            return res.status(400).json({ error: 'Invalid request parameters' });
        }

        const validClasses = ['Beam', 'Test', 'Calibration', 'Source', 'Junk', 'Cosmic'];
        if (!validClasses.includes(runClass)) {
            return res.status(400).json({ error: 'Invalid run class' });
        }

        const collection = await db.collection("runrecords");
        let updated = 0;

        for (const runNumber of runs) {
            const query = { "BOR.Run number": parseInt(runNumber) };
            const update = {
                $set: {
                    "BOR.Run Class": runClass
                }
            };

            const result = await collection.updateOne(query, update);
            if (result.modifiedCount > 0) {
                updated++;
            }
        }

        res.json({ updated });
    } catch (error) {
        console.error('Error in bulk class update:', error);
        res.status(500).json({ error: error.message });
    }
});

export default router;
