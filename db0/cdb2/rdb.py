#!/usr/bin/env python3
"""rdb.py — offline query of the RDB JSON backend (runrecords/).

Reads <dir>/NNNN/runRecord_<run>.json (or a CDB root that contains runrecords/).
Queries hit a flattened sidecar cache; JSON files are scanned only on --rebuild.

  python3 rdb.py --dir ~/data/mu3e/cdb --rebuild
  python3 rdb.py --dir ~/data/mu3e/cdb --significant
  python3 rdb.py --dir ~/data/mu3e/cdb --class cosmic --significant --dq pixel=1

Cache (inside the runrecords directory):
  .rdb.index.jsonl   one flattened record per run
  .rdb.meta.json     file count / mtime fingerprint

If the cache is missing or its format changed, the script exits and asks for
--rebuild. If it looks stale (new/removed JSON), it warns on stderr and still
queries the existing cache.

Combined class: latest RunInfo.Class if it is set, otherwise BOR "Run Class".
DataQuality / RunInfo fields are taken from the last matching Attributes entry.
"""

import argparse
import json
import os
import re
import sys
from datetime import datetime, timezone

CACHE_VERSION = 1
INDEX_NAME = ".rdb.index.jsonl"
META_NAME = ".rdb.meta.json"
UNSET = {"", "unset", "none", "null", "nan"}

DQ_KEYS = (
    "mu3e",
    "beam",
    "vertex",
    "pixel",
    "fibres",
    "tiles",
    "calibration",
    "links",
    "version",
)
DQ_CMP = re.compile(r"^([A-Za-z_]+)(==|=|!=|>=|<=|>|<)(-?\d+)$")


# ----------------------------------------------------------------------
def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def is_unset(value):
    if value is None:
        return True
    if isinstance(value, bool):
        return False
    return str(value).strip().lower() in UNSET


def flatten_key(prefix, name):
    s = re.sub(r"[^A-Za-z0-9]+", "_", str(name).strip()).strip("_").lower()
    return prefix + "_" + s if s else prefix


def to_bool(value):
    if isinstance(value, bool):
        return value
    if is_unset(value):
        return None
    s = str(value).strip().lower()
    if s in ("true", "1", "yes"):
        return True
    if s in ("false", "0", "no"):
        return False
    return None


def to_int(value):
    if value is None or isinstance(value, bool):
        return None
    if isinstance(value, int) and not isinstance(value, bool):
        return value
    if is_unset(value):
        return None
    try:
        return int(float(str(value).strip()))
    except (TypeError, ValueError):
        return None


def to_float(value):
    if value is None or isinstance(value, bool):
        return None
    if isinstance(value, (int, float)) and not isinstance(value, bool):
        return float(value)
    if is_unset(value):
        return None
    try:
        return float(str(value).strip())
    except (TypeError, ValueError):
        return None


def latest_attr(attributes, key):
    found = None
    for item in attributes or []:
        if isinstance(item, dict) and key in item and isinstance(item[key], dict):
            found = item[key]
    return found


def combine_class(bor_class, ri_class):
    if not is_unset(ri_class):
        return str(ri_class).strip()
    if not is_unset(bor_class):
        return str(bor_class).strip()
    return None


def normalize_dq(dq):
    """Latest DataQuality dict; map historical goodLinks -> links."""
    if not dq:
        return {}
    out = dict(dq)
    if "links" not in out or is_unset(out.get("links")):
        if "goodLinks" in out:
            out["links"] = out["goodLinks"]
    return out


def flatten_object(prefix, obj, dest):
    if not isinstance(obj, dict):
        return
    for key, val in obj.items():
        dest[flatten_key(prefix, key)] = val if not is_unset(val) else None


# ----------------------------------------------------------------------
def resolve_runrecords_dir(path):
    path = os.path.abspath(os.path.expanduser(path))
    if not os.path.isdir(path):
        raise SystemExit("rdb.py: not a directory: %s" % path)
    nested = os.path.join(path, "runrecords")
    if os.path.isdir(nested):
        return nested
    return path


def iter_runrecord_files(runrecords_dir):
    # block subdirs first, then flat leftover files
    blocks = []
    for name in os.listdir(runrecords_dir):
        full = os.path.join(runrecords_dir, name)
        if os.path.isdir(full) and len(name) == 4 and name.isdigit():
            blocks.append(full)
    for block in sorted(blocks):
        for name in sorted(os.listdir(block)):
            if name.startswith("runRecord_") and name.endswith(".json"):
                yield os.path.join(block, name)
    for name in sorted(os.listdir(runrecords_dir)):
        if name.startswith("runRecord_") and name.endswith(".json"):
            yield os.path.join(runrecords_dir, name)


def json_fingerprint(runrecords_dir):
    n = 0
    max_mtime = 0.0
    for path in iter_runrecord_files(runrecords_dir):
        n += 1
        try:
            m = os.path.getmtime(path)
        except OSError:
            continue
        if m > max_mtime:
            max_mtime = m
    return n, max_mtime


def index_path(runrecords_dir):
    return os.path.join(runrecords_dir, INDEX_NAME)


def meta_path(runrecords_dir):
    return os.path.join(runrecords_dir, META_NAME)


# ----------------------------------------------------------------------
def flatten_record(path, data):
    bor = data.get("BOR") if isinstance(data.get("BOR"), dict) else {}
    eor = data.get("EOR") if isinstance(data.get("EOR"), dict) else {}
    attrs = data.get("Attributes") if isinstance(data.get("Attributes"), list) else []
    dq = normalize_dq(latest_attr(attrs, "DataQuality"))
    ri = latest_attr(attrs, "RunInfo") or {}

    row = {
        "file": path,
        "run": to_int(bor.get("Run number")),
        "class": combine_class(bor.get("Run Class"), ri.get("Class")),
        "significant": to_bool(ri.get("Significant")),
    }
    flatten_object("bor", bor, row)
    flatten_object("eor", eor, row)

    for key in DQ_KEYS:
        val = dq.get(key)
        dest = "dq_" + key
        if key == "version":
            row[dest] = None if is_unset(val) else str(val)
        else:
            row[dest] = to_int(val)

    row["ri_significant"] = None if is_unset(ri.get("Significant")) else str(ri.get("Significant")).strip()
    row["ri_components"] = None if is_unset(ri.get("Components")) else str(ri.get("Components")).strip()
    row["ri_components_out"] = None if is_unset(ri.get("ComponentsOut")) else str(ri.get("ComponentsOut")).strip()
    row["ri_class"] = None if is_unset(ri.get("Class")) else str(ri.get("Class")).strip()
    row["ri_comments"] = None if is_unset(ri.get("Comments")) else str(ri.get("Comments")).strip()

    # typed copies of the usual numeric EOR / BOR fields (overwrite strings)
    row["bor_run_number"] = to_int(bor.get("Run number"))
    row["bor_mu3e_magnet"] = to_float(bor.get("Mu3e Magnet"))
    row["eor_events"] = to_int(eor.get("Events"))
    row["eor_file_size"] = to_float(eor.get("File size"))
    row["eor_uncompressed_data_size"] = to_float(eor.get("Uncompressed data size"))
    if row["run"] is None:
        row["run"] = row["bor_run_number"]
    return row


def rebuild_cache(runrecords_dir):
    rows = []
    n_err = 0
    for path in iter_runrecord_files(runrecords_dir):
        try:
            with open(path, "r") as fh:
                data = json.load(fh)
            row = flatten_record(path, data)
        except (OSError, ValueError, json.JSONDecodeError) as exc:
            eprint("rdb.py: skip %s (%s)" % (path, exc))
            n_err += 1
            continue
        rows.append(row)
    rows.sort(key=lambda r: (r.get("run") is None, r.get("run") if r.get("run") is not None else 0))

    ipath = index_path(runrecords_dir)
    with open(ipath, "w") as fh:
        for row in rows:
            fh.write(json.dumps(row, ensure_ascii=False, separators=(",", ":")) + "\n")

    n_json, max_mtime = json_fingerprint(runrecords_dir)
    meta = {
        "version": CACHE_VERSION,
        "dir": runrecords_dir,
        "n_json": n_json,
        "n_rows": len(rows),
        "n_errors": n_err,
        "max_mtime": max_mtime,
        "built_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "index": INDEX_NAME,
    }
    with open(meta_path(runrecords_dir), "w") as fh:
        json.dump(meta, fh, indent=2)
        fh.write("\n")
    eprint("rdb.py: wrote %d rows -> %s  (%d json files, %d errors)"
           % (len(rows), ipath, n_json, n_err))
    return rows, meta


def load_meta(runrecords_dir):
    path = meta_path(runrecords_dir)
    if not os.path.isfile(path):
        return None
    try:
        with open(path, "r") as fh:
            return json.load(fh)
    except (OSError, ValueError, json.JSONDecodeError):
        return None


def cache_status(runrecords_dir):
    """Return ('ok'|'stale'|'missing'|'incompatible', meta_or_None, message)."""
    meta = load_meta(runrecords_dir)
    ipath = index_path(runrecords_dir)
    if meta is None or not os.path.isfile(ipath):
        return ("missing", meta,
                "no cache — run:  python3 %s --dir %s --rebuild"
                % (os.path.basename(sys.argv[0]) or "rdb.py",
                   os.path.dirname(runrecords_dir) if os.path.basename(runrecords_dir) == "runrecords"
                   else runrecords_dir))
    if int(meta.get("version", -1)) != CACHE_VERSION:
        return ("incompatible", meta,
                "cache format v%s (need v%d) — rebuild with --rebuild"
                % (meta.get("version"), CACHE_VERSION))
    n_json, max_mtime = json_fingerprint(runrecords_dir)
    reasons = []
    if n_json != int(meta.get("n_json", -1)):
        reasons.append("json files %d vs cache %s" % (n_json, meta.get("n_json")))
    if max_mtime > float(meta.get("max_mtime", 0)) + 1e-3:
        reasons.append("JSON newer than cache")
    if reasons:
        return ("stale", meta,
                "cache stale (%s) — rebuild with --rebuild" % "; ".join(reasons))
    return ("ok", meta, None)


def load_cache(runrecords_dir):
    rows = []
    with open(index_path(runrecords_dir), "r") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue
            rows.append(json.loads(line))
    return rows


# ----------------------------------------------------------------------
def parse_run_spec(spec):
    """'226,4000-4010,5001' -> sorted unique ints."""
    runs = set()
    for part in spec.split(","):
        part = part.strip()
        if not part:
            continue
        if "-" in part:
            a, b = part.split("-", 1)
            lo, hi = int(a.strip()), int(b.strip())
            if lo > hi:
                lo, hi = hi, lo
            runs.update(range(lo, hi + 1))
        else:
            runs.add(int(part))
    return runs


def parse_dq_filter(expr):
    m = DQ_CMP.match(expr.replace(" ", ""))
    if not m:
        raise SystemExit("rdb.py: bad --dq '%s' (expected e.g. pixel=1 or pixel!=-1)" % expr)
    field, op, rhs = m.group(1), m.group(2), int(m.group(3))
    if op == "==":
        op = "="
    key = field if field.startswith("dq_") else "dq_" + field
    if key[3:] not in DQ_KEYS or key == "dq_version":
        raise SystemExit("rdb.py: unknown DQ field in --dq '%s'" % expr)
    return key, op, rhs


def cmp_num(lhs, op, rhs):
    if lhs is None:
        return False
    if op == "=":
        return lhs == rhs
    if op == "!=":
        return lhs != rhs
    if op == ">":
        return lhs > rhs
    if op == ">=":
        return lhs >= rhs
    if op == "<":
        return lhs < rhs
    if op == "<=":
        return lhs <= rhs
    return False


def apply_filters(rows, args):
    dq_filters = [parse_dq_filter(x) for x in (args.dq or [])]
    classes = [c.strip().lower() for c in (args.run_class or []) if c.strip()]
    run_set = parse_run_spec(args.run) if args.run else None
    first = args.first
    last = args.last

    out = []
    for row in rows:
        run = row.get("run")
        if run is None:
            continue
        if first is not None and run < first:
            continue
        if last is not None and run > last:
            continue
        if run_set is not None and run not in run_set:
            continue
        if args.significant and row.get("significant") is not True:
            continue
        if args.not_significant and row.get("significant") is not False:
            continue
        if classes:
            klass = (row.get("class") or "").lower()
            if klass not in classes:
                continue
        if args.min_events is not None:
            ev = row.get("eor_events")
            if ev is None or ev < args.min_events:
                continue
        if args.max_events is not None:
            ev = row.get("eor_events")
            if ev is None or ev > args.max_events:
                continue
        if args.comment:
            blob = " ".join(
                x for x in (row.get("eor_comments"), row.get("ri_comments")) if x
            ).lower()
            if args.comment.lower() not in blob:
                continue
        if args.shift:
            crew = (row.get("bor_shift_crew") or "").lower()
            if args.shift.lower() not in crew:
                continue
        if args.components:
            val = (row.get("ri_components") or "").lower()
            if args.components.lower() not in val:
                continue
        if args.components_out:
            val = (row.get("ri_components_out") or "").lower()
            if args.components_out.lower() not in val:
                continue
        ok = True
        for key, op, rhs in dq_filters:
            if not cmp_num(row.get(key), op, rhs):
                ok = False
                break
        if ok:
            out.append(row)
    return out


# ----------------------------------------------------------------------
def field_list(rows):
    if not rows:
        return []
    present = set()
    for row in rows:
        present.update(row.keys())
    present.discard("file")
    keys = []
    seen = set()
    preferred = [
        "run", "class", "significant", "bor_start_time", "eor_events",
        "dq_pixel", "dq_vertex", "ri_components", "ri_components_out",
    ]
    for k in preferred:
        if k in present and k not in seen:
            keys.append(k)
            seen.add(k)
    for k in sorted(present):
        if k not in seen:
            keys.append(k)
            seen.add(k)
    return keys


def run_numbers(rows):
    return [row["run"] for row in rows]


def print_match_summary(n_sel, n_total, args):
    if args.summary:
        eprint("rdb.py: %d / %d" % (n_sel, n_total))


def print_rows(rows, args, n_total):
    fmt = args.format
    runs = run_numbers(rows)
    if fmt == "count":
        print(len(runs))
        print_match_summary(len(runs), n_total, args)
        return
    if fmt == "line":
        for run in runs:
            print(run)
        print_match_summary(len(runs), n_total, args)
        return
    if fmt == "csv":
        print(",".join(str(run) for run in runs))
        print_match_summary(len(runs), n_total, args)
        return
    if fmt == "json":
        print(json.dumps(runs))
        print_match_summary(len(runs), n_total, args)
        return
    raise SystemExit("rdb.py: unknown --format %s" % fmt)


def print_dump(rows, n_total, args):
    for row in rows:
        path = row.get("file")
        print("# %s" % path)
        try:
            with open(path, "r") as fh:
                text = fh.read()
        except OSError as exc:
            eprint("rdb.py: cannot read %s (%s)" % (path, exc))
            continue
        sys.stdout.write(text)
        if not text.endswith("\n"):
            print()
    print_match_summary(len(rows), n_total, args)


# ----------------------------------------------------------------------
USAGE_EPILOG = """
Overview
  Offline query of the RDB JSON backend (runrecords/).  Reads
      <dir>/NNNN/runRecord_<run>.json
  or a CDB root that contains a runrecords/ subdirectory.

  Queries never scan the JSON tree.  They read a flattened sidecar cache
  written by --rebuild.  Rebuild when files were added, removed, or
  updated (for example after syncJSON --rdb).

Cache (inside the runrecords directory)
  .rdb.index.jsonl   one flattened record per run
  .rdb.meta.json     file count / mtime fingerprint

  Missing or incompatible cache: the script exits and asks for --rebuild.
  Stale cache (JSON newer or file count changed): WARNING on stderr, then
  the existing cache is still queried.  Rebuild is never automatic.

Flattened fields
  All BOR and EOR keys that occur in the files, latest DataQuality
  (goodLinks is stored as dq_links), and latest RunInfo fields
  Significant, Components, ComponentsOut, Class, Comments.

  Combined class (--class): latest RunInfo.Class if it is set, otherwise
  BOR "Run Class".  Matching is case-insensitive.  Repeat --class for OR.
  Raw values remain in the cache as ri_class and bor_run_class.

  DataQuality / RunInfo are taken from the last matching Attributes entry.

Directory
  --dir / -d    CDB root (…/cdb) or the runrecords directory itself.
  If --dir is omitted, the environment variable MU3E_CDB is used.

Filters (AND unless noted)
  --significant / --not-significant
  --class NAME          combined class; repeatable = OR
  --dq EXPR             latest DQ, e.g. pixel=1 or pixel!=-1 (repeatable).
                        Fields: mu3e beam vertex pixel fibres tiles
                        calibration links
                        Operators: =  ==  !=  >  >=  <  <=
  --min-events / --max-events    EOR Events
  -f / -l               first / last run number (inclusive)
  -r / --r / --run LIST 226,4000-4010  (commas and ranges)
  --comment TEXT        substring on EOR Comments or RunInfo.Comments
  --shift TEXT          substring on BOR Shift crew
  --components / --components-out    substring on those RunInfo fields

Output (stdout = matching run numbers unless dump/count)
  --format csv          comma-separated run numbers, one line (default)
           json         JSON array of run numbers
           line         one run number per line
           dump         original JSON of matching runs
           count        number of matches only
  --summary             print "rdb.py: N / TOTAL" to stderr after the result
  --list-fields         print cache column names and exit
  --rebuild             rebuild cache; with filters, query afterwards

Examples
  python3 rdb.py --dir ~/data/mu3e/cdb --rebuild
  python3 rdb.py --dir ~/data/mu3e/cdb --significant
  python3 rdb.py --dir ~/data/mu3e/cdb --class cosmic --significant --dq 'pixel!=-1'
  python3 rdb.py --dir ~/data/mu3e/cdb -f 4000 -l 5000 --min-events 10000
  python3 rdb.py --dir ~/data/mu3e/cdb --class beam --format json
  python3 rdb.py --dir ~/data/mu3e/cdb --significant --format line
  python3 rdb.py --dir ~/data/mu3e/cdb -r 226 --format dump
"""


class _HelpParser(argparse.ArgumentParser):
    """Print the full help text on argparse errors (unknown/missing flags)."""

    def error(self, message):
        self.print_help(sys.stderr)
        self.exit(2, "\nrdb.py: error: %s\n" % message)


def build_parser():
    p = _HelpParser(
        prog="rdb.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=(
            "rdb.py — offline query of RDB runrecords JSON (flattened sidecar cache).\n"
            "Run with no arguments, -h, or --help to print this message."
        ),
        epilog=USAGE_EPILOG,
        add_help=False,
        allow_abbrev=False,
    )
    p.add_argument("-h", "--help", action="help",
                   help="print this help message and exit")
    p.add_argument("--dir", "-d", default=os.environ.get("MU3E_CDB"),
                   metavar="DIR",
                   help="CDB root or runrecords directory (default: $MU3E_CDB)")
    p.add_argument("--rebuild", action="store_true",
                   help="rebuild the flattened cache from the JSON files")
    p.add_argument("--significant", action="store_true",
                   help="keep runs whose latest RunInfo.Significant is true")
    p.add_argument("--not-significant", action="store_true",
                   help="keep runs whose latest RunInfo.Significant is false")
    p.add_argument("--class", dest="run_class", action="append", default=[],
                   metavar="NAME",
                   help="combined class (RunInfo.Class if set, else BOR). Repeatable = OR")
    p.add_argument("--dq", action="append", default=[], metavar="EXPR",
                   help="DataQuality filter, e.g. pixel=1 or pixel!=-1 (repeatable)")
    p.add_argument("--min-events", type=int, default=None, metavar="N",
                   help="minimum EOR Events")
    p.add_argument("--max-events", type=int, default=None, metavar="N",
                   help="maximum EOR Events")
    p.add_argument("-f", dest="first", type=int, default=None, metavar="N",
                   help="first run number (inclusive)")
    p.add_argument("-l", dest="last", type=int, default=None, metavar="N",
                   help="last run number (inclusive)")
    p.add_argument("-r", "--r", "--run", dest="run", default=None, metavar="LIST",
                   help="run list/ranges, e.g. 226,4000-4010")
    p.add_argument("--comment", default=None, metavar="TEXT",
                   help="substring on EOR Comments or RunInfo.Comments")
    p.add_argument("--shift", default=None, metavar="TEXT",
                   help="substring on BOR Shift crew")
    p.add_argument("--components", default=None, metavar="TEXT",
                   help="substring on RunInfo.Components")
    p.add_argument("--components-out", dest="components_out", default=None,
                   metavar="TEXT",
                   help="substring on RunInfo.ComponentsOut")
    p.add_argument("--format", choices=("csv", "json", "line", "dump", "count"),
                   default="csv", metavar="FMT",
                   help="run-list format: csv (default), json, line; or dump / count")
    p.add_argument("--summary", action="store_true",
                   help="print match count to stderr (N / TOTAL)")
    p.add_argument("--list-fields", action="store_true",
                   help="print cache column names and exit")
    return p


def main(argv=None):
    argv = sys.argv[1:] if argv is None else list(argv)
    parser = build_parser()
    if not argv:
        parser.print_help(sys.stdout)
        return 2
    args = parser.parse_args(argv)
    if not args.dir:
        parser.print_help(sys.stderr)
        eprint("\nrdb.py: error: --dir is required (or set environment MU3E_CDB)")
        return 2
    runrecords_dir = resolve_runrecords_dir(args.dir)

    if args.rebuild:
        rows, _meta = rebuild_cache(runrecords_dir)
    else:
        status, _meta, msg = cache_status(runrecords_dir)
        if status in ("missing", "incompatible"):
            raise SystemExit("rdb.py: " + msg)
        if status == "stale":
            eprint("rdb.py: WARNING " + msg)
        rows = load_cache(runrecords_dir)

    if args.list_fields:
        if not rows:
            raise SystemExit("rdb.py: cache is empty")
        for k in field_list(rows):
            print(k)
        return 0

    query = (
        args.significant or args.not_significant or args.run_class or args.dq
        or args.min_events is not None or args.max_events is not None
        or args.first is not None or args.last is not None or args.run
        or args.comment or args.shift or args.components or args.components_out
    )
    if args.rebuild and not query:
        return 0

    selected = apply_filters(rows, args)
    if args.format == "dump":
        print_dump(selected, len(rows), args)
    else:
        print_rows(selected, args, len(rows))
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        try:
            sys.stdout.close()
        except Exception:
            pass
        sys.exit(0)
    except KeyboardInterrupt:
        sys.exit(130)
