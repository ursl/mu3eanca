#!/usr/bin/env node
/**
 * Standalone RelVal dashboard (same routes as mounted under /relval on the main REST app).
 * Uses this package's dependencies only; run from db1/rest after npm install.
 *
 *   RELVAL_BASEDIR=/data/relval PORT=8787 node scripts/serveRelvalDashboard.mjs
 */

import "../loadEnvironment.mjs";
import path from "path";
import express from "express";
import { createRelvalRouter } from "../lib/relvalRouter.mjs";

const PORT = Number(process.env.PORT || 8787);
const raw = process.env.RELVAL_BASEDIR;
if (!raw) {
  console.error("RELVAL_BASEDIR must be set");
  process.exit(1);
}

const base = path.resolve(raw);
const app = express();
app.use(createRelvalRouter(base));

app.listen(PORT, () => {
  console.log(`RelVal dashboard: http://localhost:${PORT}/`);
  console.log(`Reading setups from: ${base}`);
});
