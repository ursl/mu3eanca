import fs from "fs";
import path from "path";
import express from "express";
import { fileURLToPath } from "url";

import {
  buildCombinedSnakemakeLog,
  listSetups,
  safeJoin,
} from "./relvalCore.mjs";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const relvalNotConfiguredPayload = {
  error: "RELVAL_BASEDIR is not set on the server.",
  hint:
    "Add RELVAL_BASEDIR to db1/rest/.env (absolute path to the relval tree: same value as mu3e_relval_basedir in prompt/relval/config.yaml) and restart.",
};

/**
 * Express router: API + static UI + file download under the same mount path.
 * Mount with e.g. app.use("/relval", createRelvalRouter(baseDir)).
 *
 * @param {string | null | undefined} relvalBaseDir - Root directory of relval outputs (mu3e-* workdirs), or null if unset.
 */
export function createRelvalRouter(relvalBaseDir) {
  const trimmed =
    relvalBaseDir == null ? "" : String(relvalBaseDir).trim();
  const base = trimmed === "" ? null : path.resolve(trimmed);
  const publicDir = path.join(__dirname, "../public/relval");
  const router = express.Router();

  function sendNotConfigured(_req, res) {
    res.status(503).json(relvalNotConfiguredPayload);
  }

  if (base == null) {
    router.get("/api/setups", sendNotConfigured);
    router.get("/api/setups/:setupName/snakemake-log", sendNotConfigured);
    router.get("/api/setups/:setupName", sendNotConfigured);
    router.get("/relval-file", sendNotConfigured);
  } else {
    router.get("/api/setups", (_req, res) => {
      const setups = listSetups(base);
      res.json({
        relvalBaseDir: base,
        setupCount: setups.length,
        setups,
      });
    });

    router.get("/api/setups/:setupName/snakemake-log", (req, res) => {
      const setupName = req.params.setupName;
      const setup = listSetups(base).find((s) => s.name === setupName);
      if (!setup) {
        res.status(404).json({ error: `Unknown setup: ${setupName}` });
        return;
      }
      const combined = buildCombinedSnakemakeLog(base, setup.setupDir, setup.name);
      res.json(combined);
    });

    router.get("/api/setups/:setupName", (req, res) => {
      const setupName = req.params.setupName;
      const setup = listSetups(base).find((s) => s.name === setupName);
      if (!setup) {
        res.status(404).json({ error: `Unknown setup: ${setupName}` });
        return;
      }
      res.json(setup);
    });

    router.get("/relval-file", (req, res) => {
      const relPath = typeof req.query.path === "string" ? req.query.path : "";
      const fullPath = safeJoin(base, relPath);
      if (!fullPath || !fs.existsSync(fullPath)) {
        res.status(404).send("Not found");
        return;
      }
      const ext = path.extname(fullPath).toLowerCase();
      const contentType =
        ext === ".pdf"
          ? "application/pdf"
          : ext === ".txt" || ext === ".log"
            ? "text/plain; charset=utf-8"
            : "application/octet-stream";
      try {
        const body = fs.readFileSync(fullPath);
        res.setHeader("Content-Type", contentType);
        res.send(body);
      } catch {
        res.status(500).send("Failed to read file");
      }
    });
  }

  router.use(express.static(publicDir));

  return router;
}
