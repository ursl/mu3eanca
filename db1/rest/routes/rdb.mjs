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
    const DEFAULT_PAGE_SIZE = 1000;
    const MAX_PAGE_SIZE = 5000; // Maximum allowed page size

    console.log("----------------------------------------------------");
    console.log("req.query: " + JSON.stringify(req.query));
    console.log("serving from RDB / " + req.params.id);

    // Parse query parameters with onlySignificant defaulting to "yes"
    const nruns = req.query.nRun ? Number(req.query.nRun) : -1; // Legacy parameter, kept for backward compatibility
    const minrun = Number(req.query.minRun) || -1;
    const maxrun = Number(req.query.maxRun) || -1;
    const onlySignificant = req.query.onlySignificant === "no" ? "no" : "yes";  // Default to "yes" unless explicitly set to "no"
    const starttime = req.query.startTime;
    const stoptime = req.query.stopTime;
    const runClass = req.query.runClass;
    const comment = req.query.comment;  // Add comment parameter
    
    // Pagination parameters
    const page = Math.max(1, Number(req.query.page) || 1);
    const pageSize = Math.min(MAX_PAGE_SIZE, Math.max(1, Number(req.query.pageSize) || DEFAULT_PAGE_SIZE));
    
    // Data Quality filter parameters
    const dqMu3e = req.query.dqMu3e;
    const dqBeam = req.query.dqBeam;
    const dqVtx = req.query.dqVtx;
    const dqPix = req.query.dqPix;
    const dqFib = req.query.dqFib;
    const dqTil = req.query.dqTil;
    const dqLks = req.query.dqLks;

    console.log("Query params:", { nruns, minrun, maxrun, onlySignificant, starttime, stoptime, runClass, comment, dqMu3e, dqBeam, dqVtx, dqPix, dqFib, dqTil, dqLks });

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
                },
                lastDataQuality: {
                    $arrayElemAt: [
                        {
                            $filter: {
                                input: "$Attributes",
                                as: "attr",
                                cond: { $eq: [{ $type: "$$attr.DataQuality" }, "object"] }
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

        // Data Quality filters - build match conditions for each field
        const dqMatchConditions = [];
        const dqFieldMap = {
            'dqMu3e': 'mu3e',
            'dqBeam': 'beam',
            'dqVtx': 'vertex',
            'dqPix': 'pixel',
            'dqFib': 'fibres',
            'dqTil': 'tiles',
            'dqLks': 'links'
        };

        // Process each Data Quality filter
        for (const [paramName, fieldName] of Object.entries(dqFieldMap)) {
            const filterValue = req.query[paramName];
            if (filterValue !== undefined && filterValue !== '') {
                if (filterValue === "-1") {
                    // Unset: lastDataQuality is null, or field doesn't exist, or is not "1" or "0" (as string or number)
                    dqMatchConditions.push({
                        $or: [
                            { "lastDataQuality": null },
                            { [`lastDataQuality.DataQuality.${fieldName}`]: { $exists: false } },
                            { [`lastDataQuality.DataQuality.${fieldName}`]: { $nin: ["1", "0", 1, 0] } }
                        ]
                    });
                } else if (filterValue === "notbad") {
                    // Not Bad: field is not "0" or 0 (matches Good, Unset, or any value except Bad)
                    dqMatchConditions.push({
                        [`lastDataQuality.DataQuality.${fieldName}`]: { $nin: ["0", 0] }
                    });
                } else {
                    // Good (1) or Bad (0) - field must exist and match the value
                    // Match both string and number versions since MongoDB can store either
                    const numValue = parseInt(filterValue, 10);
                    dqMatchConditions.push({
                        [`lastDataQuality.DataQuality.${fieldName}`]: { $in: [filterValue, numValue] }
                    });
                }
            }
        }

        // Apply Data Quality filters if any are specified
        // All conditions must be met (AND logic) - explicitly use $and for all cases
        if (dqMatchConditions.length > 0) {
            // Always use $and to ensure explicit AND logic, even for single condition
            pipeline.push({ $match: { $and: dqMatchConditions } });
        }

        // Sort by run number descending
        pipeline.push({ $sort: { "BOR.Run number": -1 } });

        // Get total count efficiently - build count pipeline (same filters, no sort/limit/skip)
        const countPipeline = [];
        for (let i = 0; i < pipeline.length; i++) {
            if (pipeline[i].$sort) break; // Stop before sort
            countPipeline.push(pipeline[i]);
        }
        
        // Get total count and data in parallel for better performance
        const skip = (page - 1) * pageSize;
        
        // Build data pipeline with pagination
        const dataPipeline = [...pipeline];
        if (nruns > 0) {
            // Legacy mode
            dataPipeline.push({ $limit: nruns });
        } else {
            dataPipeline.push({ $skip: skip });
            dataPipeline.push({ $limit: pageSize });
        }

        console.log("Aggregation pipeline:", JSON.stringify(dataPipeline, null, 2));
        console.log(`Pagination: page=${page}, pageSize=${pageSize}, skip=${skip}`);
        
        // Execute count and data queries in parallel
        const [countResult, dataResult] = await Promise.all([
            collection.aggregate([...countPipeline, { $count: "total" }]).toArray(),
            collection.aggregate(dataPipeline).toArray()
        ]);
        
        const totalCount = countResult.length > 0 ? countResult[0].total : 0;
        const actualResults = dataResult;
        
        console.log("Query successful, found", actualResults.length, "results out of", totalCount, "total");
        
        // Calculate pagination info with accurate total count
        const totalPages = totalCount > 0 ? Math.ceil(totalCount / pageSize) : 1;
        const hasNextPage = page < totalPages;
        const hasPrevPage = page > 1;
        
        // Calculate pagination info - always provide pagination object
        // Ensure all values are valid numbers
        const pagination = {
            currentPage: Math.max(1, parseInt(page) || 1),
            pageSize: Math.max(1, parseInt(pageSize) || 1000),
            totalCount: Math.max(0, parseInt(totalCount) || 0),
            totalPages: Math.max(1, parseInt(totalPages) || 1),
            hasNextPage: Boolean(hasNextPage),
            hasPrevPage: Boolean(hasPrevPage)
        };
        
        console.log("Pagination object:", JSON.stringify(pagination));
        
        // Check Accept header - only prefer JSON if it explicitly requests JSON and NOT HTML
        const acceptHeader = req.headers.accept || '';
        const prefersJson = acceptHeader.includes('application/json') && 
                           !acceptHeader.includes('text/html') &&
                           (acceptHeader.split(',')[0].trim().includes('application/json') || 
                            acceptHeader === 'application/json');
        
        const resultSize = JSON.stringify(actualResults).length;
        const MAX_RENDER_SIZE = 50 * 1024 * 1024; // 50MB limit for rendering (increased from 10MB)
        
        console.log(`Accept header: ${acceptHeader}`);
        console.log(`Prefers JSON: ${prefersJson}`);
        console.log(`Result size: ${resultSize} bytes (${Math.round(resultSize/1024/1024)}MB)`);
        
        // Only return JSON if explicitly requested or if result is extremely large
        if (prefersJson) {
            console.log("JSON explicitly requested, returning JSON");
            return res.json({
                message: 'JSON format requested',
                count: actualResults.length,
                data: actualResults
            });
        }
        
        if (resultSize > MAX_RENDER_SIZE) {
            console.log(`Result too large (${resultSize} bytes), returning JSON with message`);
            return res.json({
                message: `Result set too large (${actualResults.length} records, ${Math.round(resultSize/1024/1024)}MB). Please use filters to reduce the result set.`,
                count: actualResults.length,
                data: actualResults
            });
        }
        
        // Try to render the HTML template
        console.log("Attempting to render HTML template...");
        
        // Explicitly set content type to HTML
        res.setHeader('Content-Type', 'text/html; charset=utf-8');
        
        try {
            res.render('index', {
                'data': actualResults,
                'onlySignificant': onlySignificant || 'yes',  // Default to 'yes' if not specified
                'runClass': runClass || '',  // Pass the run class to the template
                'comment': comment || '',  // Pass the comment to the template
                'dqMu3e': dqMu3e || '',
                'dqBeam': dqBeam || '',
                'dqVtx': dqVtx || '',
                'dqPix': dqPix || '',
                'dqFib': dqFib || '',
                'dqTil': dqTil || '',
                'dqLks': dqLks || '',
                'pagination': pagination,
                'page': page,
                'pageSize': pageSize
            });
            console.log("Template render call completed - response will be sent automatically");
        } catch (renderError) {
            console.error("Synchronous error during render call:", renderError);
            console.error("Render error stack:", renderError.stack);
            // Fallback to JSON if template rendering fails
            console.log("Falling back to JSON response due to render error");
            return res.json({
                error: "Template rendering failed",
                message: renderError.message,
                count: result.length,
                data: result
            });
        }
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
// -- Get all run numbers (with optional filters)
router.get("/allRunNumbers", async (req, res) => {
    console.log("serving from RDB /allRunNumbers with filters:", JSON.stringify(req.query));
    let collection = await db.collection("runrecords");
    
    // Parse query parameters (same as main route)
    const minrun = Number(req.query.minRun) || -1;
    const maxrun = Number(req.query.maxRun) || -1;
    const onlySignificant = req.query.onlySignificant === "no" ? "no" : "yes";
    const starttime = req.query.startTime;
    const stoptime = req.query.stopTime;
    const runClass = req.query.runClass;
    const comment = req.query.comment;
    const dqMu3e = req.query.dqMu3e;
    const dqBeam = req.query.dqBeam;
    const dqVtx = req.query.dqVtx;
    const dqPix = req.query.dqPix;
    const dqFib = req.query.dqFib;
    const dqTil = req.query.dqTil;
    const dqLks = req.query.dqLks;
    
    try {
        // Build the same aggregation pipeline as the main route (without pagination)
        let pipeline = [];
        
        // Filter by run number range if specified
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

        // Add fields for filtering (same as main route)
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
                        -1
                    ]
                },
                lastDataQuality: {
                    $arrayElemAt: [
                        {
                            $filter: {
                                input: "$Attributes",
                                as: "attr",
                                cond: { $eq: [{ $type: "$$attr.DataQuality" }, "object"] }
                            }
                        },
                        -1
                    ]
                }
            }
        });

        // Apply filters (same logic as main route)
        if (onlySignificant === "yes") {
            pipeline.push({
                $match: {
                    "lastRunInfo.RunInfo.Significant": "true"
                }
            });
        }

        if (runClass) {
            pipeline.push({
                $match: {
                    "BOR.Run Class": runClass
                }
            });
        }

        if (comment) {
            pipeline.push({
                $match: {
                    $or: [
                        { "lastRunInfo.RunInfo.Comments": { 
                            $regex: comment,
                            $options: 'i'
                        }},
                        { "EOR.Comments": { 
                            $regex: comment,
                            $options: 'i'
                        }}
                    ]
                }
            });
        }

        // Data Quality filters
        const dqMatchConditions = [];
        const dqFieldMap = {
            'dqMu3e': 'mu3e',
            'dqBeam': 'beam',
            'dqVtx': 'vertex',
            'dqPix': 'pixel',
            'dqFib': 'fibres',
            'dqTil': 'tiles',
            'dqLks': 'links'
        };

        for (const [paramName, fieldName] of Object.entries(dqFieldMap)) {
            const filterValue = req.query[paramName];
            if (filterValue !== undefined && filterValue !== '') {
                if (filterValue === "-1") {
                    dqMatchConditions.push({
                        $or: [
                            { "lastDataQuality": null },
                            { [`lastDataQuality.DataQuality.${fieldName}`]: { $exists: false } },
                            { [`lastDataQuality.DataQuality.${fieldName}`]: { $nin: ["1", "0", 1, 0] } }
                        ]
                    });
                } else if (filterValue === "notbad") {
                    dqMatchConditions.push({
                        [`lastDataQuality.DataQuality.${fieldName}`]: { $nin: ["0", 0] }
                    });
                } else {
                    const numValue = parseInt(filterValue, 10);
                    dqMatchConditions.push({
                        [`lastDataQuality.DataQuality.${fieldName}`]: { $in: [filterValue, numValue] }
                    });
                }
            }
        }

        if (dqMatchConditions.length > 0) {
            pipeline.push({ $match: { $and: dqMatchConditions } });
        }

        // Project only the run number
        pipeline.push({
            $project: {
                runNumber: "$BOR.Run number"
            }
        });

        // Sort by run number descending
        pipeline.push({ $sort: { runNumber: -1 } });

        const results = await collection.aggregate(pipeline).toArray();
        const runNumbers = results.map(record => record.runNumber);
        
        console.log(`Returning ${runNumbers.length} run numbers`);
        res.json(runNumbers);
    } catch (error) {
        console.error("Error in allRunNumbers:", error);
        res.status(500).json({ error: error.message });
    }
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
