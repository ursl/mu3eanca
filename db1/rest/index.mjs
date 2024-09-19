import express from "express";
import cors from "cors";
import path from "path";
import "./loadEnvironment.mjs";
import "express-async-errors";
import bodyParser from "body-parser";

//import db from "./db/conn.mjs";
import db from "./db/conn.mjs";

import cdb from "./routes/cdb.mjs";
import rdb from "./routes/rdb.mjs";

import { dirname } from 'path';
import { fileURLToPath } from 'url';
const __dirname = dirname(fileURLToPath(import.meta.url));

const PORT = process.env.PORT || 5050;
const app = express();

app.use(cors());
app.use(express.json());
app.use(bodyParser.json());

app.locals.isFiltered = false;

// Load the /posts routes
app.use("/cdb", cdb);
app.use("/rdb", rdb);

app.set('view engine', 'ejs');
app.set("views", path.join(__dirname, "views"));

// Global error handling
app.use((err, _req, res, next) => {
  res.status(500).send("Uh oh! An unexpected error occured.")
})

// start the Express server
app.listen(PORT, () => {
  console.log(`Server is running on port: ${PORT}`);
});
