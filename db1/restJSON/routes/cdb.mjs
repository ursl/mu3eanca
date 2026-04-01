import express from "express";
import fs from "fs/promises";
import path from "path";
import os from "os";
import { execFile } from "child_process";
import { promisify } from "util";
import { dirname } from "path";
import { fileURLToPath } from "url";

const execFileAsync = promisify(execFile);

const router = express.Router();
const __dirname = dirname(fileURLToPath(import.meta.url));

function cdbRootDir() {
  const root = process.env.CDBROOTDIR || process.env.CDBROOT || "";
  return root.trim();
}

function payloadSubPathFromHash(hash) {
  const prefix = "tag_";
  const suffix = "_iov_";
  if (!hash || !hash.startsWith(prefix)) return hash;
  const pos = hash.lastIndexOf(suffix);
  if (pos < prefix.length) return hash;
  const tag = hash.substring(prefix.length, pos);
  const iovStr = hash.substring(pos + suffix.length);
  const iov = Number.parseInt(iovStr, 10);
  if (!Number.isFinite(iov) || iov < 0) return hash;
  const block = Math.floor(iov / 1000).toString().padStart(4, "0");
  return path.join(tag, block, hash);
}

async function readJsonFile(filePath) {
  const raw = await fs.readFile(filePath, "utf8");
  return JSON.parse(raw);
}

async function listFilesInDir(dirPath) {
  const entries = await fs.readdir(dirPath, { withFileTypes: true });
  return entries.filter(e => e.isFile()).map(e => e.name).sort();
}

async function listFilesRecursive(dirPath) {
  const out = [];
  async function walk(current) {
    const entries = await fs.readdir(current, { withFileTypes: true });
    for (const ent of entries) {
      const full = path.join(current, ent.name);
      if (ent.isDirectory()) {
        await walk(full);
      } else if (ent.isFile()) {
        out.push(full);
      }
    }
  }
  await walk(dirPath);
  out.sort();
  return out;
}

/**
 * Best-effort: is CDBROOTDIR resolved path on an sshfs (FUSE) mount?
 * Linux: findmnt or /proc/mounts. macOS/BSD: parse `mount`.
 */
async function detectCdbRootMount(resolvedRoot) {
  const out = { sshfs: false, fstype: "", method: "none", detail: "" };
  if (!resolvedRoot || process.platform === "win32") {
    out.method = "unsupported";
    return out;
  }
  const norm = path.resolve(resolvedRoot).replace(/\/+$/, "") || "/";

  if (process.platform === "linux") {
    try {
      const { stdout } = await execFileAsync("findmnt", ["-n", "-o", "FSTYPE", "--target", norm], {
        timeout: 5000,
      });
      const fst = stdout.trim().toLowerCase();
      out.fstype = fst || "";
      out.method = "findmnt";
      out.sshfs = fst.includes("sshfs");
      return out;
    } catch {
      /* fall through */
    }
    try {
      const text = await fs.readFile("/proc/mounts", "utf8");
      let best = { len: -1, fstype: "", device: "" };
      for (const line of text.split("\n")) {
        if (!line) continue;
        const parts = line.split(" ");
        if (parts.length < 4) continue;
        const device = parts[0];
        const mountpoint = parts[1].replace(/\\040/g, " ");
        const fstype = parts[2];
        const mp = mountpoint.replace(/\/+$/, "") || "/";
        if (norm === mp || norm.startsWith(mp + "/")) {
          if (mp.length > best.len) best = { len: mp.length, fstype, device };
        }
      }
      if (best.len < 0) {
        out.method = "proc";
        out.detail = "no matching mount";
        return out;
      }
      out.fstype = best.fstype;
      out.method = "proc";
      const f = best.fstype.toLowerCase();
      out.sshfs = f === "fuse.sshfs" || f.includes("sshfs");
      if (!out.sshfs && f === "fuse" && /[:@]/.test(best.device)) out.sshfs = true;
      return out;
    } catch (e) {
      out.method = "proc-error";
      out.detail = e.message;
      return out;
    }
  }

  try {
    const { stdout } = await execFileAsync("mount", [], { timeout: 8000 });
    let best = { len: -1, line: "", opts: "" };
    for (const line of stdout.split("\n")) {
      const m = line.match(/\s+on\s+(.+?)\s+\(([^)]+)\)\s*$/);
      if (!m) continue;
      const mpRaw = m[1].trim();
      const opts = m[2].toLowerCase();
      let mpNorm;
      try {
        mpNorm = await fs.realpath(mpRaw);
      } catch {
        mpNorm = path.resolve(mpRaw);
      }
      mpNorm = path.resolve(mpNorm).replace(/\/+$/, "") || "/";
      if (norm === mpNorm || norm.startsWith(mpNorm + "/")) {
        if (mpNorm.length > best.len) best = { len: mpNorm.length, line, opts };
      }
    }
    if (best.len < 0) {
      out.method = "mount";
      out.detail = "no matching mount";
      return out;
    }
    out.method = "mount";
    const low = best.line.toLowerCase();
    const dev = best.line.split(" on ")[0].trim();
    const fuseLike = best.opts.includes("macfuse") || best.opts.includes("osxfuse") || low.includes("fuse");
    const sshLike = /@/.test(dev) && /:/.test(dev);
    out.sshfs = (fuseLike && sshLike) || low.includes("sshfs");
    out.fstype = best.opts.split(",")[0].trim() || "";
    return out;
  } catch (e) {
    out.method = "mount-error";
    out.detail = e.message;
    return out;
  }
}

function checkRoot(res) {
  const root = cdbRootDir();
  if (!root) {
    res.status(500).json({ error: "CDBROOTDIR is not set" });
    return "";
  }
  return root;
}

router.get("/", (_req, res) => {
  res.sendFile(path.join(__dirname, "../public/cdb.html"));
});

router.get("/findAll/globaltags", async (_req, res) => {
  const root = checkRoot(res);
  if (!root) return;
  try {
    const dir = path.join(root, "globaltags");
    const names = await listFilesInDir(dir);
    const docs = [];
    for (const name of names) {
      try {
        const doc = await readJsonFile(path.join(dir, name));
        docs.push(doc);
      } catch (_e) {
        // Skip malformed files
      }
    }
    res.status(200).json(docs);
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});

router.get("/findOne/globaltags/:id", async (req, res) => {
  const root = checkRoot(res);
  if (!root) return;
  try {
    const doc = await readJsonFile(path.join(root, "globaltags", req.params.id));
    res.status(200).json(doc);
  } catch (_e) {
    res.status(404).send("Not found");
  }
});

router.get("/findOne/tags/:id", async (req, res) => {
  const root = checkRoot(res);
  if (!root) return;
  try {
    const doc = await readJsonFile(path.join(root, "tags", req.params.id));
    res.status(200).json(doc);
  } catch (_e) {
    res.status(404).send("Not found");
  }
});

router.get("/findOne/payloads/:id", async (req, res) => {
  const root = checkRoot(res);
  if (!root) return;
  const hash = req.params.id;
  const subpath = payloadSubPathFromHash(hash);
  try {
    const full1 = path.join(root, "payloads", subpath);
    try {
      const doc = await readJsonFile(full1);
      return res.status(200).json(doc);
    } catch (_e) {
      const full2 = path.join(root, "payloads", hash);
      const doc = await readJsonFile(full2);
      return res.status(200).json(doc);
    }
  } catch (_e) {
    return res.status(404).send("Not found");
  }
});

router.get("/findTagsByGlobaltag/:globaltag", async (req, res) => {
  const root = checkRoot(res);
  if (!root) return;
  try {
    const gtDoc = await readJsonFile(path.join(root, "globaltags", req.params.globaltag));
    const gtTags = Array.isArray(gtDoc.tags) ? gtDoc.tags : [];
    const results = [];
    for (const tag of gtTags) {
      try {
        const tdoc = await readJsonFile(path.join(root, "tags", tag));
        results.push({
          tag: tdoc.tag || tag,
          description: tdoc.description || "",
          comment: typeof tdoc.comment === "string" ? tdoc.comment : "",
          iovs: Array.isArray(tdoc.iovs) ? tdoc.iovs : [],
        });
      } catch (_e) {
        // Skip missing tag files
      }
    }
    res.status(200).json(results);
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});

router.get("/findPayloadsByTag/:tag", async (req, res) => {
  const root = checkRoot(res);
  if (!root) return;
  const tag = req.params.tag;
  const limit = Number.parseInt(req.query.limit, 10) || 1000;
  try {
    let tagDoc = { tag, iovs: [] };
    try {
      tagDoc = await readJsonFile(path.join(root, "tags", tag));
    } catch (_e) {
      // keep fallback
    }

    const baseDir = path.join(root, "payloads", tag);
    let files = [];
    try {
      files = await listFilesRecursive(baseDir);
    } catch (_e) {
      files = [];
    }

    const payloads = [];
    for (const file of files) {
      try {
        const doc = await readJsonFile(file);
        payloads.push({
          hash: doc.hash || "",
          date: doc.date || "",
          comment: doc.comment || "",
          schema: doc.schema || ""
        });
      } catch (_e) {
        // Skip malformed payload file
      }
    }

    payloads.sort((a, b) => (a.date < b.date ? 1 : a.date > b.date ? -1 : 0));
    const trimmed = payloads.slice(0, limit);

    res.status(200).json({
      tag: tagDoc.tag || tag,
      iovs: Array.isArray(tagDoc.iovs) ? tagDoc.iovs : [],
      payloads: trimmed,
      totalCount: payloads.length,
      returnedCount: trimmed.length,
      hasMore: payloads.length > limit
    });
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});

router.get("/findAll/detconfigsSummary", async (_req, res) => {
  res.status(200).json([]);
});

router.get("/hostname", async (_req, res) => {
  const root = cdbRootDir();
  let resolved = "";
  const cdbMount = {
    sshfs: false,
    fstype: "",
    method: "unconfigured",
    detail: "",
  };
  try {
    if (root) {
      try {
        resolved = await fs.realpath(root);
      } catch {
        resolved = root;
      }
      Object.assign(cdbMount, await detectCdbRootMount(resolved));
    }
  } catch (e) {
    cdbMount.detail = e.message;
    cdbMount.method = "error";
  }
  res.status(200).json({
    hostname: os.hostname(),
    cdbRoot: root,
    cdbRootResolved: resolved,
    cdbMount,
  });
});

router.get("/debugRoot", async (_req, res) => {
  const root = cdbRootDir();
  const out = {
    CDBROOTDIR: process.env.CDBROOTDIR || "",
    CDBROOT: process.env.CDBROOT || "",
    resolvedRoot: root || "",
    exists: false,
    globaltagsCount: 0,
    tagsCount: 0,
    payloadTagDirCount: 0
  };
  try {
    if (!root) return res.status(200).json(out);
    const st = await fs.stat(root);
    out.exists = st.isDirectory();
    const [gts, tags, payloads] = await Promise.all([
      listFilesInDir(path.join(root, "globaltags")).catch(() => []),
      listFilesInDir(path.join(root, "tags")).catch(() => []),
      fs.readdir(path.join(root, "payloads"), { withFileTypes: true }).catch(() => [])
    ]);
    out.globaltagsCount = gts.length;
    out.tagsCount = tags.length;
    out.payloadTagDirCount = payloads.filter(e => e.isDirectory()).length;
    return res.status(200).json(out);
  } catch (e) {
    out.error = e.message;
    return res.status(200).json(out);
  }
});

export default router;
