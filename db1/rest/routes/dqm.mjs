import express from "express";
import fs from "fs";

const router = express.Router();

// -- Provide a single run for jsroot display
router.get("/run", async (req, res) => {
  console.log("serving /dqmsrv");
});


// -- Provide a single run for jsroot display
router.get("/run/:id", async (req, res) => {
    console.log("serving /dqm/run/" + req.params.id);
    
    fs.readFile('./jsroot/index.htm', function (err, data) {
        if (err == null ) {
            res.writeHead(200, {'Content-Type': 'text/html'});
            res.write(data);
            res.end();
        }
    });
});

export default router;
