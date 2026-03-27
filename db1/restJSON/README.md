# restJSON

Standalone file-backed CDB browser service.

## Purpose

Serve the `cdb.html` UI against JSON files in a local CDB tree (no MongoDB required).

Expected CDB tree under `CDBROOTDIR`:

- `globaltags/`
- `tags/`
- `payloads/`

## Run

```bash
cd db1/restJSON
npm install
export CDBROOTDIR=/path/to/cdb-root
npm start
```

Open:

- `http://localhost:5051/cdb`

Optional compatibility mode:

- `http://localhost:5051/cdb?backend=json`

