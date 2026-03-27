import express from "express";
import cors from "cors";
import path from "path";
import "./loadEnvironment.mjs";
import "express-async-errors";
import bodyParser from "body-parser";

import cdb from "./routes/cdb.mjs";

import { dirname } from "path";
import { fileURLToPath } from "url";
const __dirname = dirname(fileURLToPath(import.meta.url));

const PORT = process.env.PORT || 5051;
const app = express();

app.use(cors());
app.use(express.json());
app.use(bodyParser.json());

app.use(express.static(path.join(__dirname, "public")));

// File-backed CDB endpoints
app.use("/cdb", cdb);
// Optional alias to keep compatibility with backend=json URLs
app.use("/cdbjson", cdb);

// Global error handling
app.use((err, _req, res, _next) => {
  console.error(err);
  res.status(500).send("Uh oh! An unexpected error occured.");
});

app.listen(PORT, () => {
  console.log(`restJSON server is running on port: ${PORT}`);
});
