#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const http = require("http");
const { URL } = require("url");

const PORT = Number(process.env.PORT || 8787);
const RELVAL_BASEDIR =
  process.env.RELVAL_BASEDIR || "/Users/ursl/data/mu3e/relval";
const PUBLIC_DIR = path.join(__dirname, "public");

function safeJoin(baseDir, subPath) {
  const normalized = path.normalize(path.join(baseDir, subPath));
  if (!normalized.startsWith(baseDir)) return null;
  return normalized;
}

function readTextIfExists(filePath) {
  try {
    return fs.readFileSync(filePath, "utf8");
  } catch {
    return null;
  }
}

function parseTsv(tsvText) {
  if (!tsvText) return [];
  const lines = tsvText.split(/\r?\n/).filter((l) => l.trim().length > 0);
  if (lines.length < 2) return [];
  const header = lines[0].split("\t");
  return lines.slice(1).map((line) => {
    const cols = line.split("\t");
    const row = {};
    for (let i = 0; i < header.length; i += 1) {
      row[header[i]] = cols[i] ?? "";
    }
    return row;
  });
}

function summarizeRows(rows) {
  const statusCounts = {};
  const planCounts = {};
  for (const row of rows) {
    const status = row.status || "unknown";
    const plan = row.plan || "unknown";
    statusCounts[status] = (statusCounts[status] || 0) + 1;
    planCounts[plan] = (planCounts[plan] || 0) + 1;
  }
  return {
    totalRows: rows.length,
    statusCounts,
    planCounts,
    okCount: statusCounts.ok || 0,
    pendingCount: planCounts["update pending"] || 0,
  };
}

function listPdfFiles(dirPath) {
  try {
    return fs
      .readdirSync(dirPath, { withFileTypes: true })
      .filter((d) => d.isFile() && d.name.toLowerCase().endsWith(".pdf"))
      .map((d) => path.join(dirPath, d.name))
      .sort((a, b) => a.localeCompare(b));
  } catch {
    return [];
  }
}

function relToBase(absPath) {
  return path.relative(RELVAL_BASEDIR, absPath).replaceAll(path.sep, "/");
}

function collectSnakemakeLogs(setupDir, setupName) {
  const candidateDirs = [];
  candidateDirs.push(path.join(setupDir, ".snakemake", "log"));
  if (setupName.startsWith("mu3e-")) {
    const setupSafe = setupName.slice("mu3e-".length);
    candidateDirs.push(path.join(RELVAL_BASEDIR, "setups", setupSafe, ".snakemake", "log"));
  }

  const logMap = new Map();
  for (const logDir of candidateDirs) {
    try {
      for (const d of fs.readdirSync(logDir, { withFileTypes: true })) {
        if (!d.isFile() || !d.name.endsWith(".snakemake.log")) continue;
        const absPath = path.join(logDir, d.name);
        let mtimeMs = 0;
        try {
          mtimeMs = fs.statSync(absPath).mtimeMs;
        } catch {
          mtimeMs = 0;
        }
        // Key by absolute path so we can merge from multiple dirs.
        logMap.set(absPath, {
          name: d.name,
          relPath: relToBase(absPath),
          mtimeMs,
        });
      }
    } catch {
      // ignore missing candidate dir
    }
  }

  const files = [...logMap.values()].sort((a, b) => b.mtimeMs - a.mtimeMs);
  return { logDirs: candidateDirs, files };
}

function buildCombinedSnakemakeLog(setupDir, setupName) {
  const snk = collectSnakemakeLogs(setupDir, setupName);
  if (!snk.files.length) {
    return {
      setupName,
      fileCount: 0,
      files: [],
      text: "",
    };
  }

  // Concatenate oldest -> newest for better chronology.
  const files = [...snk.files].sort((a, b) => a.mtimeMs - b.mtimeMs);
  const chunks = [];
  for (const f of files) {
    const absPath = safeJoin(RELVAL_BASEDIR, f.relPath);
    if (!absPath) continue;
    const content = readTextIfExists(absPath);
    if (content == null) continue;
    chunks.push(`===== ${f.relPath} =====\n${content}`);
  }

  return {
    setupName,
    fileCount: files.length,
    files: files.map((f) => ({ name: f.name, relPath: f.relPath })),
    text: chunks.join("\n\n"),
  };
}

function collectCompareOutputs(setupDir) {
  const compareRoot = path.join(setupDir, "run", "output", "compare");
  let compareDirs = [];
  try {
    compareDirs = fs
      .readdirSync(compareRoot, { withFileTypes: true })
      .filter((d) => d.isDirectory())
      .map((d) => path.join(compareRoot, d.name))
      .sort((a, b) => a.localeCompare(b));
  } catch {
    return { compareRoot, scenarios: [], totals: { runComparePdfCount: 0, histocomparePdfCount: 0 } };
  }

  const scenarios = compareDirs.map((dirPath) => {
    const dirName = path.basename(dirPath);
    const runCompareSummaryPdfs = listPdfFiles(dirPath).filter((f) =>
      path.basename(f).startsWith("summary-"),
    );
    const histocomparePdfs = listPdfFiles(dirPath).filter((f) =>
      path.basename(f).startsWith("histocompare-"),
    );
    const logFiles = [];
    try {
      for (const d of fs.readdirSync(dirPath, { withFileTypes: true })) {
        if (d.isFile() && d.name.toLowerCase().endsWith(".log")) {
          logFiles.push(path.join(dirPath, d.name));
        }
      }
    } catch {
      // ignore
    }
    logFiles.sort((a, b) => a.localeCompare(b));
    const runCompareLogs = logFiles.filter((f) =>
      path.basename(f).startsWith("summary-"),
    );
    const histocompareLogs = logFiles.filter((f) =>
      path.basename(f).startsWith("histocompare-"),
    );

    return {
      dirName,
      relativeDir: relToBase(dirPath),
      runCompareSummaryPdfs: runCompareSummaryPdfs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(f),
      })),
      histocomparePdfs: histocomparePdfs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(f),
      })),
      runCompareLogs: runCompareLogs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(f),
      })),
      histocompareLogs: histocompareLogs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(f),
      })),
    };
  });

  const runComparePdfCount = scenarios.reduce(
    (acc, s) => acc + s.runCompareSummaryPdfs.length,
    0,
  );
  const histocomparePdfCount = scenarios.reduce(
    (acc, s) => acc + s.histocomparePdfs.length,
    0,
  );

  return {
    compareRoot,
    scenarios,
    totals: { runComparePdfCount, histocomparePdfCount },
  };
}

function listSetups() {
  let entries = [];
  try {
    entries = fs.readdirSync(RELVAL_BASEDIR, { withFileTypes: true });
  } catch {
    return [];
  }

  const setups = entries
    .filter((e) => e.isDirectory() && e.name.startsWith("mu3e-"))
    .map((dirent) => {
      const name = dirent.name;
      const setupDir = path.join(RELVAL_BASEDIR, name);
      const statusDir = path.join(setupDir, "status");
      const summaryPath = path.join(statusDir, "summary.tsv");
      const detailedPath = path.join(statusDir, "detailed-summary.tsv");

      const summaryText = readTextIfExists(summaryPath);
      const summaryRows = parseTsv(summaryText);
      const stats = summarizeRows(summaryRows);

      const statusDirExists = fs.existsSync(statusDir);
      const summaryExists = fs.existsSync(summaryPath);
      const detailedExists = fs.existsSync(detailedPath);
      const compare = collectCompareOutputs(setupDir);
      const snakemakeLogs = collectSnakemakeLogs(setupDir, name);

      return {
        name,
        setupDir,
        statusDir,
        files: {
          statusDirExists,
          summaryExists,
          detailedExists,
          summaryPath,
          detailedPath,
        },
        stats,
        compare,
        snakemakeLogs,
        updatedAt: summaryExists
          ? fs.statSync(summaryPath).mtime.toISOString()
          : null,
        rows: summaryRows,
      };
    })
    .sort((a, b) => a.name.localeCompare(b.name));

  return setups;
}

function sendJson(res, statusCode, data) {
  res.writeHead(statusCode, { "Content-Type": "application/json; charset=utf-8" });
  res.end(JSON.stringify(data, null, 2));
}

function sendStatic(reqPath, res) {
  const file = reqPath === "/" ? "/index.html" : reqPath;
  const fullPath = path.normalize(path.join(PUBLIC_DIR, file));
  if (!fullPath.startsWith(PUBLIC_DIR)) {
    res.writeHead(403);
    res.end("Forbidden");
    return;
  }

  let body = null;
  try {
    body = fs.readFileSync(fullPath);
  } catch {
    res.writeHead(404);
    res.end("Not found");
    return;
  }

  const ext = path.extname(fullPath);
  const contentType =
    ext === ".html"
      ? "text/html; charset=utf-8"
      : ext === ".css"
        ? "text/css; charset=utf-8"
        : ext === ".js"
          ? "application/javascript; charset=utf-8"
          : "application/octet-stream";

  res.writeHead(200, { "Content-Type": contentType });
  res.end(body);
}

const server = http.createServer((req, res) => {
  const urlObj = new URL(req.url, `http://${req.headers.host}`);

  if (urlObj.pathname === "/relval-file") {
    const relPath = urlObj.searchParams.get("path") || "";
    const fullPath = safeJoin(RELVAL_BASEDIR, relPath);
    if (!fullPath || !fs.existsSync(fullPath)) {
      res.writeHead(404);
      res.end("Not found");
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
      res.writeHead(200, { "Content-Type": contentType });
      res.end(body);
    } catch {
      res.writeHead(500);
      res.end("Failed to read file");
    }
    return;
  }

  if (urlObj.pathname === "/api/setups") {
    const setups = listSetups();
    sendJson(res, 200, {
      relvalBaseDir: RELVAL_BASEDIR,
      setupCount: setups.length,
      setups,
    });
    return;
  }

  if (urlObj.pathname.startsWith("/api/setups/")) {
    const tail = urlObj.pathname.replace("/api/setups/", "");
    if (tail.endsWith("/snakemake-log")) {
      const setupName = decodeURIComponent(tail.replace(/\/snakemake-log$/, ""));
      const setup = listSetups().find((s) => s.name === setupName);
      if (!setup) {
        sendJson(res, 404, { error: `Unknown setup: ${setupName}` });
        return;
      }
      const combined = buildCombinedSnakemakeLog(setup.setupDir, setup.name);
      sendJson(res, 200, combined);
      return;
    }

    const setupName = decodeURIComponent(tail);
    const setup = listSetups().find((s) => s.name === setupName);
    if (!setup) {
      sendJson(res, 404, { error: `Unknown setup: ${setupName}` });
      return;
    }
    sendJson(res, 200, setup);
    return;
  }

  sendStatic(urlObj.pathname, res);
});

server.listen(PORT, () => {
  console.log(`RelVal dashboard listening on http://localhost:${PORT}`);
  console.log(`Reading setups from: ${RELVAL_BASEDIR}`);
});
