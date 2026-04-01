#!/usr/bin/env python3
from pathlib import Path
import sys


def must_replace(text: str, old: str, new: str, label: str) -> str:
    if old not in text:
        raise RuntimeError(f"overlay failed: pattern not found for {label}")
    return text.replace(old, new, 1)


def apply_cdb_html_overlay(path: Path) -> None:
    t = path.read_text()

    t = must_replace(
        t,
        "    <script>\n",
        "    <script>\n"
        "        const queryParams = new URLSearchParams(window.location.search);\n"
        "        const backendMode = (queryParams.get(\"backend\") || \"mongo\").toLowerCase();\n"
        "        const isJsonBackend = backendMode === \"json\";\n"
        "        const CDB_API_BASE = isJsonBackend ? \"/cdbjson\" : \"/cdb\";\n"
        "\n",
        "insert backend switch block",
    )

    replacements = [
        ("fetch('/cdb/findAll/globaltags')", "fetch(`${CDB_API_BASE}/findAll/globaltags`)"),
        (
            "fetch(`/cdb/findTagsByGlobaltag/${encodeURIComponent(globaltag)}`)",
            "fetch(`${CDB_API_BASE}/findTagsByGlobaltag/${encodeURIComponent(globaltag)}`)",
        ),
        ("fetch(`/cdb/findPayloadsByTag/${tag}`)", "fetch(`${CDB_API_BASE}/findPayloadsByTag/${tag}`)"),
        ("fetch(`/cdb/findOne/payloads/${encodeURIComponent(hash)}`)", "fetch(`${CDB_API_BASE}/findOne/payloads/${encodeURIComponent(hash)}`)"),
        ("fetch('/cdb/findAll/detconfigsSummary')", "fetch(`${CDB_API_BASE}/findAll/detconfigsSummary`)"),
        ("fetch(`/cdb/downloadTag?tag=${encodeURIComponent(tag)}`)", "fetch(`${CDB_API_BASE}/downloadTag?tag=${encodeURIComponent(tag)}`)"),
        ("fetch(`/cdb/deleteDetconfigTag?tag=${encodeURIComponent(tag)}`, {", "fetch(`${CDB_API_BASE}/deleteDetconfigTag?tag=${encodeURIComponent(tag)}`, {"),
        ("fetch('/cdb/uploadMany', {", "fetch(`${CDB_API_BASE}/uploadMany`, {"),
        ("fetch('/cdb/hostname')", "fetch(`${CDB_API_BASE}/hostname`)"),
    ]
    for old, new in replacements:
        t = must_replace(t, old, new, f"replace {old}")

    t = must_replace(
        t,
        "        async function loadDetconfigsSummary() {\n"
        "            setLoading('detconfigs-loading', true);\n",
        "        async function loadDetconfigsSummary() {\n"
        "            if (isJsonBackend) {\n"
        "                const tbody = document.querySelector('#detconfigs-table tbody');\n"
        "                tbody.innerHTML = '<tr><td colspan=\"3\" class=\"text-center\">detconfigs upload/management disabled in JSON backend mode</td></tr>';\n"
        "                return;\n"
        "            }\n"
        "            setLoading('detconfigs-loading', true);\n",
        "inject detconfigs skip block",
    )

    t = must_replace(
        t,
        "                        hostnameDisplay.textContent = data.hostname || window.location.hostname;\n",
        "                        const host = data.hostname || window.location.hostname || \"Unknown\";\n"
        "                        const root = data.cdbRoot || \"unset\";\n"
        "                        const m = data.cdbMount || {};\n"
        "                        let sshfsLine = \"SSHFS: n/a\";\n"
        "                        if (root && root !== \"unset\") {\n"
        "                            if (m.sshfs === true) sshfsLine = \"SSHFS: yes\";\n"
        "                            else if (m.sshfs === false) sshfsLine = \"SSHFS: no\" + (m.fstype ? ` (${m.fstype})` : \"\");\n"
        "                            else sshfsLine = \"SSHFS: unknown\" + (m.detail ? ` (${m.detail})` : \"\");\n"
        "                        }\n"
        "                        hostnameDisplay.textContent = `${host} (CDBROOT: ${root}) · ${sshfsLine}`;\n",
        "hostname with cdb root + sshfs",
    )
    t = must_replace(
        t,
        "                hostnameDisplay.textContent = window.location.hostname || 'Unknown';\n",
        "                hostnameDisplay.textContent = `${window.location.hostname || 'Unknown'} (CDBROOT: unknown)`;\n",
        "hostname fallback 1",
    )
    t = must_replace(
        t,
        "                hostnameDisplay.textContent = window.location.hostname || 'Unknown';\n",
        "                hostnameDisplay.textContent = `${window.location.hostname || 'Unknown'} (CDBROOT: unknown)`;\n",
        "hostname fallback 2",
    )

    t = must_replace(
        t,
        "        document.addEventListener('DOMContentLoaded', () => {\n"
        "            loadGlobaltags();\n",
        "        document.addEventListener('DOMContentLoaded', () => {\n"
        "            if (isJsonBackend) {\n"
        "                const uploadOpenBtn = document.querySelector('[data-bs-target=\"#uploadModal\"]');\n"
        "                if (uploadOpenBtn) uploadOpenBtn.style.display = 'none';\n"
        "            }\n"
        "            loadGlobaltags();\n",
        "hide upload button in json mode",
    )

    path.write_text(t)


def apply_route_overlay(path: Path) -> None:
    t = path.read_text()
    t = must_replace(
        t,
        '  const root = process.env.CDBROOTDIR || "";\n',
        '  const root = process.env.CDBROOTDIR || process.env.CDBROOT || "";\n',
        "CDBROOT fallback",
    )
    hostname_block = (
        "router.get(\"/hostname\", async (_req, res) => {\n"
        "  const root = cdbRootDir();\n"
        "  let resolved = \"\";\n"
        "  const cdbMount = {\n"
        "    sshfs: false,\n"
        "    fstype: \"\",\n"
        "    method: \"unconfigured\",\n"
        "    detail: \"\",\n"
        "  };\n"
        "  try {\n"
        "    if (root) {\n"
        "      try {\n"
        "        resolved = await fs.realpath(root);\n"
        "      } catch {\n"
        "        resolved = root;\n"
        "      }\n"
        "      Object.assign(cdbMount, await detectCdbRootMount(resolved));\n"
        "    }\n"
        "  } catch (e) {\n"
        "    cdbMount.detail = e.message;\n"
        "    cdbMount.method = \"error\";\n"
        "  }\n"
        "  res.status(200).json({\n"
        "    hostname: os.hostname(),\n"
        "    cdbRoot: root,\n"
        "    cdbRootResolved: resolved,\n"
        "    cdbMount,\n"
        "  });\n"
        "});\n"
    )
    debug_block = (
        "\n"
        "router.get(\"/debugRoot\", async (_req, res) => {\n"
        "  const root = cdbRootDir();\n"
        "  const out = {\n"
        "    CDBROOTDIR: process.env.CDBROOTDIR || \"\",\n"
        "    CDBROOT: process.env.CDBROOT || \"\",\n"
        "    resolvedRoot: root || \"\",\n"
        "    exists: false,\n"
        "    globaltagsCount: 0,\n"
        "    tagsCount: 0,\n"
        "    payloadTagDirCount: 0\n"
        "  };\n"
        "  try {\n"
        "    if (!root) return res.status(200).json(out);\n"
        "    const st = await fs.stat(root);\n"
        "    out.exists = st.isDirectory();\n"
        "    const [gts, tags, payloads] = await Promise.all([\n"
        "      listFilesInDir(path.join(root, \"globaltags\")).catch(() => []),\n"
        "      listFilesInDir(path.join(root, \"tags\")).catch(() => []),\n"
        "      fs.readdir(path.join(root, \"payloads\"), { withFileTypes: true }).catch(() => [])\n"
        "    ]);\n"
        "    out.globaltagsCount = gts.length;\n"
        "    out.tagsCount = tags.length;\n"
        "    out.payloadTagDirCount = payloads.filter(e => e.isDirectory()).length;\n"
        "    return res.status(200).json(out);\n"
        "  } catch (e) {\n"
        "    out.error = e.message;\n"
        "    return res.status(200).json(out);\n"
        "  }\n"
        "});\n"
    )
    t = must_replace(
        t,
        hostname_block + "\nexport default router;\n",
        hostname_block + debug_block + "\nexport default router;\n",
        "append debugRoot after hostname (cdbjson already has async hostname + cdbMount)",
    )
    path.write_text(t)


def main() -> int:
    rest_dir = Path(__file__).resolve().parents[1]
    root = rest_dir.parent
    target_html = root / "restJSON" / "public" / "cdb.html"
    target_route = root / "restJSON" / "routes" / "cdb.mjs"

    apply_cdb_html_overlay(target_html)
    apply_route_overlay(target_route)
    print("Applied restJSON overlay patches.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise
