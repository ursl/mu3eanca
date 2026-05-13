import express from "express";
import cors from "cors";
import path from "path";
import "./loadEnvironment.mjs";
import "express-async-errors";
import bodyParser from "body-parser";

import db from "./db/conn.mjs";

// -- conditions database
import cdb from "./routes/cdb.mjs";

// -- run database
import rdb from "./routes/rdb.mjs";

// -- provide access to detConfigs
import ddb from "./routes/ddb.mjs";

// -- provide access to DQM plots
import dqm from "./routes/dqm.mjs";

import { createRelvalRouter } from "./lib/relvalRouter.mjs";

import { dirname } from "path";
import { fileURLToPath } from 'url';
const __dirname = dirname(fileURLToPath(import.meta.url));

const PORT = process.env.PORT || 5050;
const app = express();

app.use(cors());
app.use(express.json());
app.use(bodyParser.json());

// Serve static files from the public directory
app.use(express.static(path.join(__dirname, 'public')));

app.locals.isFiltered = false;

// Load the /posts routes
app.use("/cdb", cdb);
app.use("/rdb", rdb);
app.use("/ddb", ddb);
app.use("/dqm", dqm);

// RelVal UI: /relval/ must use a trailing slash so relative "api/setups" resolves correctly.
app.get("/relval", (_req, res) => res.redirect(302, "/relval/"));

const relvalRaw = process.env.RELVAL_BASEDIR;
const relvalResolved =
  relvalRaw != null && String(relvalRaw).trim() !== ""
    ? path.resolve(String(relvalRaw).trim())
    : null;
app.use("/relval", createRelvalRouter(relvalResolved));

app.set('view engine', 'ejs');
app.set("views", path.join(__dirname, "views"));

// Global error handling
app.use((err, _req, res, next) => {
  res.status(500).send("Uh oh! An unexpected error occured.")
})

// start the Express server
app.listen(PORT, () => {
  console.log(`Server is running on port: ${PORT}`);
  console.log(`RelVal dashboard: http://localhost:${PORT}/relval/`);
  if (relvalResolved) {
    console.log(`  RELVAL_BASEDIR=${relvalResolved}`);
  } else {
    console.log(
      "  RELVAL_BASEDIR not set — UI loads but API returns 503 until you set it in .env",
    );
  }
});
