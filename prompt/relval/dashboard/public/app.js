async function loadSetups() {
  const res = await fetch("/api/setups");
  if (!res.ok) {
    throw new Error(`Failed to fetch setups: HTTP ${res.status}`);
  }
  return res.json();
}

function badgeClass(setup) {
  if (!setup.files.summaryExists) return "missing";
  if (setup.stats.pendingCount > 0) return "warn";
  return "ok";
}

function badgeText(setup) {
  if (!setup.files.summaryExists) return "no summary";
  if (setup.stats.pendingCount > 0) return "update pending";
  return "up to date";
}

function pdfLink(relPath, label) {
  const href = `/relval-file?path=${encodeURIComponent(relPath)}`;
  return `<a href="${href}" target="_blank" rel="noopener">${label}</a>`;
}

function parseReleasesFromSetup(setupName) {
  const suffix = setupName.startsWith("mu3e-relval_")
    ? setupName.slice("mu3e-relval_".length)
    : setupName;
  const match = suffix.match(/^mu3e-([^_]+)_(.+)$/);
  if (!match) {
    return { newRelease: suffix, refRelease: "n/a" };
  }
  return { newRelease: match[1], refRelease: "n/a" };
}

function scenarioFromCompareDirName(dirName) {
  const [scenario] = dirName.split("__");
  return scenario || dirName;
}

function refReleaseFromCompareDirName(dirName) {
  // Example:
  // conf18_threelayer__mu3e-relval_mu3e-v6.5_mcidealv6.5__vs__mu3e-relval_mu3e-v6.4.4_mcidealv6.5
  //                                            ^^^^^^^^^ reference release
  const m = String(dirName).match(/__vs__mu3e-relval_mu3e-([^_]+)_[^_]+$/);
  return m ? m[1] : null;
}

function parseReleaseVersion(release) {
  const m = String(release).match(/^v(\d+)\.(\d+)(?:\.(\d+))?(?:pre(\d+))?$/);
  if (!m) return null;
  return {
    major: Number(m[1]),
    minor: Number(m[2]),
    patch: m[3] ? Number(m[3]) : 0,
    pre: m[4] ? Number(m[4]) : null,
  };
}

function compareReleaseDesc(aRelease, bRelease) {
  const a = parseReleaseVersion(aRelease);
  const b = parseReleaseVersion(bRelease);
  if (!a && !b) return String(bRelease).localeCompare(String(aRelease));
  if (!a) return 1;
  if (!b) return -1;
  if (a.major !== b.major) return b.major - a.major;
  if (a.minor !== b.minor) return b.minor - a.minor;
  if (a.patch !== b.patch) return b.patch - a.patch;
  // Stable release is newer than pre-release.
  if (a.pre == null && b.pre != null) return -1;
  if (a.pre != null && b.pre == null) return 1;
  if (a.pre == null && b.pre == null) return 0;
  return b.pre - a.pre;
}

function renderSetupLogs(setup) {
  const details = document.getElementById("details");
  const title = `<h3>Logs for ${setup.name}</h3>`;
  const sections = [];
  for (const s of setup.compare.scenarios) {
    const scenario = scenarioFromCompareDirName(s.dirName);
    const runCompareLogs = (s.runCompareLogs || [])
      .map((l) => `<li>${pdfLink(l.relPath, `${scenario} (runCompare)`)}</li>`)
      .join("");
    const histocompareLogs = (s.histocompareLogs || [])
      .map((l) => `<li>${pdfLink(l.relPath, `${scenario} (histocompare)`)}</li>`)
      .join("");
    if (runCompareLogs || histocompareLogs) {
      sections.push(`
        <h4>${scenario}</h4>
        <ul>
          ${runCompareLogs}
          ${histocompareLogs}
        </ul>
      `);
    }
  }
  if (sections.length === 0) {
    details.innerHTML = `${title}<p><em>No compare logs available for this setup.</em></p>`;
    return;
  }
  details.innerHTML = `${title}${sections.join("")}`;
}

function renderSetupsTable(model) {
  const tbody = document.getElementById("setups-tbody");
  tbody.innerHTML = "";

  const sortedSetups = [...model.setups].sort((a, b) => {
    const ra = parseReleasesFromSetup(a.name);
    const rb = parseReleasesFromSetup(b.name);
    return compareReleaseDesc(ra.newRelease, rb.newRelease);
  });

  let firstRow = null;
  let firstSetup = null;
  for (const setup of sortedSetups) {
    const tr = document.createElement("tr");
    const releases = parseReleasesFromSetup(setup.name);
    const firstCompareDir = setup.compare.scenarios?.[0]?.dirName || "";
    const refRelease = refReleaseFromCompareDirName(firstCompareDir) || "n/a";
    const runCompareLinks = setup.compare.scenarios
      .flatMap((s) =>
        s.runCompareSummaryPdfs.map((p) =>
          pdfLink(p.relPath, scenarioFromCompareDirName(s.dirName)),
        ),
      )
      .join(" ");
    const histocompareLinks = setup.compare.scenarios
      .flatMap((s) =>
        s.histocomparePdfs.map((p) =>
          pdfLink(p.relPath, scenarioFromCompareDirName(s.dirName)),
        ),
      )
      .join(" ");

    tr.innerHTML = `
      <td><strong>${releases.newRelease}</strong></td>
      <td>${refRelease}</td>
      <td><span class="badge ${badgeClass(setup)}">${badgeText(setup)}</span></td>
      <td>${setup.updatedAt ?? "n/a"}</td>
      <td class="pdf-links">${histocompareLinks || "<span>none</span>"}</td>
      <td class="pdf-links">${runCompareLinks || "<span>none</span>"}</td>
    `;

    tr.addEventListener("click", () => {
      for (const row of tbody.querySelectorAll("tr")) row.classList.remove("selected");
      tr.classList.add("selected");
      renderSetupLogs(setup);
    });
    tbody.appendChild(tr);

    if (!firstRow) {
      firstRow = tr;
      firstSetup = setup;
    }
  }

  if (firstRow && firstSetup) {
    firstRow.classList.add("selected");
    renderSetupLogs(firstSetup);
  } else {
    document.getElementById("details").innerHTML =
      "<em>No setups found.</em>";
  }
}

async function refresh() {
  const model = await loadSetups();
  document.getElementById(
    "base-dir",
  ).textContent = `Scanning setups in: ${model.relvalBaseDir}`;
  renderSetupsTable(model);
}

document.getElementById("refresh-btn").addEventListener("click", () => {
  refresh().catch((err) => {
    alert(err.message);
  });
});

refresh().catch((err) => {
  document.getElementById("details").textContent = err.stack || String(err);
});
