/**
 * Filesystem logic for the RelVal dashboard (served from db1/rest).
 * All paths use the given relvalBaseDir (caller should pass path.resolve(...) if needed).
 */

import fs from "fs";
import path from "path";

export function safeJoin(baseDir, subPath) {
  const normalized = path.normalize(path.join(baseDir, subPath));
  if (!normalized.startsWith(baseDir)) return null;
  return normalized;
}

export function readTextIfExists(filePath) {
  try {
    return fs.readFileSync(filePath, "utf8");
  } catch {
    return null;
  }
}

export function parseTsv(tsvText) {
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

export function summarizeRows(rows) {
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

function relToBase(relvalBaseDir, absPath) {
  return path.relative(relvalBaseDir, absPath).replaceAll(path.sep, "/");
}

export function collectSnakemakeLogs(relvalBaseDir, setupDir, setupName) {
  const candidateDirs = [];
  candidateDirs.push(path.join(setupDir, ".snakemake", "log"));
  if (setupName.startsWith("mu3e-")) {
    const setupSafe = setupName.slice("mu3e-".length);
    candidateDirs.push(
      path.join(relvalBaseDir, "setups", setupSafe, ".snakemake", "log"),
    );
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
        logMap.set(absPath, {
          name: d.name,
          relPath: relToBase(relvalBaseDir, absPath),
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

export function buildCombinedSnakemakeLog(relvalBaseDir, setupDir, setupName) {
  const snk = collectSnakemakeLogs(relvalBaseDir, setupDir, setupName);
  if (!snk.files.length) {
    return {
      setupName,
      fileCount: 0,
      files: [],
      text: "",
    };
  }

  const files = [...snk.files].sort((a, b) => a.mtimeMs - b.mtimeMs);
  const chunks = [];
  for (const f of files) {
    const absPath = safeJoin(relvalBaseDir, f.relPath);
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

export function collectCompareOutputs(relvalBaseDir, setupDir) {
  const compareRoot = path.join(setupDir, "run", "output", "compare");
  let compareDirs = [];
  try {
    compareDirs = fs
      .readdirSync(compareRoot, { withFileTypes: true })
      .filter((d) => d.isDirectory())
      .map((d) => path.join(compareRoot, d.name))
      .sort((a, b) => a.localeCompare(b));
  } catch {
    return {
      compareRoot,
      scenarios: [],
      totals: { runComparePdfCount: 0, histocomparePdfCount: 0 },
    };
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
      relativeDir: relToBase(relvalBaseDir, dirPath),
      runCompareSummaryPdfs: runCompareSummaryPdfs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(relvalBaseDir, f),
      })),
      histocomparePdfs: histocomparePdfs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(relvalBaseDir, f),
      })),
      runCompareLogs: runCompareLogs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(relvalBaseDir, f),
      })),
      histocompareLogs: histocompareLogs.map((f) => ({
        name: path.basename(f),
        relPath: relToBase(relvalBaseDir, f),
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

export function listSetups(relvalBaseDir) {
  let entries = [];
  try {
    entries = fs.readdirSync(relvalBaseDir, { withFileTypes: true });
  } catch {
    return [];
  }

  const setups = entries
    .filter((e) => e.isDirectory() && e.name.startsWith("mu3e-"))
    .map((dirent) => {
      const name = dirent.name;
      const setupDir = path.join(relvalBaseDir, name);
      const statusDir = path.join(setupDir, "status");
      const summaryPath = path.join(statusDir, "summary.tsv");
      const detailedPath = path.join(statusDir, "detailed-summary.tsv");

      const summaryText = readTextIfExists(summaryPath);
      const summaryRows = parseTsv(summaryText);
      const stats = summarizeRows(summaryRows);

      const statusDirExists = fs.existsSync(statusDir);
      const summaryExists = fs.existsSync(summaryPath);
      const detailedExists = fs.existsSync(detailedPath);
      const compare = collectCompareOutputs(relvalBaseDir, setupDir);
      const snakemakeLogs = collectSnakemakeLogs(relvalBaseDir, setupDir, name);

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
