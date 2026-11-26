# report.py  -  pull ONE W11 REPORT block out of a serial log, check that it holds together,
#               and say - per figure - which numbers may go on the record sheet.
#
# Usage:  python report.py run1-hierarchy.txt
#         python report.py "platformio-device-monitor-*.log"     (must match exactly one file)
#         python report.py some.log --block 2                    (a log with more than one block)
#
# Exit code 0 = the block is valid; every figure whose line says "ok" may be copied.
# Exit code 2 = the block is not valid and the reason is printed. Nothing is guessed,
#               defaulted or silently filled in.
#
# A figure the sketch REFUSED stays refused here: it is listed with the sketch's reason and
# it is not a measurement. A block in which every figure is refused is not a valid run.
# This script also re-does the sketch's own arithmetic (spread, floor, us-per-inference) and
# refuses a block that disagrees with itself - that is the one failure the board cannot see.
#
# Standard library only. No numpy, nothing to install.
import sys
import glob

BEGIN = "=== W11 REPORT ==="
END = "=== END ==="

# Keys every sketch prints before its tests, in w11_reportBegin().
COMMON = ["sketch", "cpu_mhz", "repeats", "spread_max_pct", "min_cycles", "dcache_bytes",
          "line_bytes", "psram_found_api", "psram_bytes", "internal_largest"]

# Per sketch: the timed tests (each has <name>_status / <name>_ns_per_item / <name>_spread_pct /
# <name>_checksum / <name>_first_ns_per_item) and the tier each one claims to have read.
TESTS = {
    "1_hierarchy": {
        "tests": {"sram_seq": "SRAM", "sram_rand": "SRAM",
                  "psram128k_seq": "PSRAM", "psram128k_rand": "PSRAM",
                  "psram1m_seq": "PSRAM", "psram1m_rand": "PSRAM",
                  "flash128k_seq": "FLASH", "flash128k_rand": "FLASH",
                  "flash1m_seq": "FLASH", "flash1m_rand": "FLASH",
                  "control_noload": None},
        "tier_key": lambda t: t.rsplit("_", 1)[0],      # sram_seq -> sram_tier
        "extra": ["words_128k", "words_1m", "malloc_2k_tier", "malloc_64k_tier"],
        "balance": ["sram_balance", "psram128k_balance", "psram1m_balance",
                    "flash128k_balance", "flash1m_balance"],
        "per": "word",
    },
    "2_dense": {
        "tests": {"f32_sram": "SRAM", "f32_psram": "PSRAM", "f32_flash": "FLASH",
                  "s8_sram": "SRAM", "s8_psram": "PSRAM", "s8_flash": "FLASH"},
        "tier_key": lambda t: t,                        # f32_sram -> f32_sram_tier
        "extra": ["n_in", "n_out", "macs_per_inference", "weights_bytes_f32",
                  "weights_bytes_s8", "scale_bytes_s8", "table_check"],
        "balance": ["f32_outputs_identical", "s8_outputs_identical", "table_check"],
        "per": "MAC",
        "derived": ("_us_per_inference", "macs_per_inference", 1 / 1000.0),
    },
    "3_sparse": {
        "tests": {"flash1m_lookup": "FLASH", "flash16k_lookup": "FLASH",
                  "psram1m_lookup": "PSRAM", "psram16k_lookup": "PSRAM",
                  "sram16k_lookup": "SRAM"},
        "tier_key": lambda t: t,
        "extra": ["rows_per_token", "row_bytes", "bytes_per_token", "tokens_per_pass",
                  "sram_holds_table"],
        "balance": [],
        "per": "token",
        "derived": ("_us_per_token", None, 1 / 1000.0),
    },
}


class Invalid(Exception):
    """Raised with a one-paragraph reason whenever a block must not be used."""


# ----------------------------------------------------------------------------- parsing

def blocks(path):
    """Every REPORT block in the file, in order: (dict, complete) pairs."""
    import os
    if not os.path.isfile(path):
        sys.exit("file not found: %s -- did you move the log into w11/ and rename it?" % path)
    out, cur = [], None
    with open(path, encoding="utf-8", errors="replace") as f:
        for raw in f:
            line = raw.strip()
            if line.startswith(BEGIN):
                if cur is not None:
                    out.append((cur, False))
                cur = {}
                continue
            if line.startswith(END):
                if cur is not None:
                    out.append((cur, True))
                cur = None
                continue
            if cur is not None and "=" in line:
                k, v = line.split("=", 1)
                cur[k.strip()] = v.strip()
    if cur is not None:
        out.append((cur, False))
    return out


def resolve_path(arg):
    """One file, exactly. A pattern that matches several files is refused, not guessed at."""
    hits = sorted(glob.glob(arg)) if any(c in arg for c in "*?[") else [arg]
    if len(hits) == 0:
        raise Invalid("no file matches %r. Did the monitor write a .log in this folder?" % arg)
    if len(hits) > 1:
        raise Invalid("%r matches %d files:\n    %s\n  Name the one you mean." %
                      (arg, len(hits), "\n    ".join(hits)))
    return hits[0]


def parse(path, block_no=None):
    bl = blocks(path)
    if not bl:
        raise Invalid("no '%s' block in %s. Did the run finish? Press RST with the monitor "
                      "already open and wait for '%s'." % (BEGIN, path, END))
    if block_no is None:
        if len(bl) > 1:
            raise Invalid("%s holds %d report blocks, and this script will not pick one for you. "
                          "Take a clean log (one run per file), or say which: --block N (1..%d)."
                          % (path, len(bl), len(bl)))
        d, complete = bl[0]
    else:
        if not 1 <= block_no <= len(bl):
            raise Invalid("--block %d, but %s holds %d block(s)." % (block_no, path, len(bl)))
        d, complete = bl[block_no - 1]
    if not complete:
        raise Invalid("the report block in %s starts but never reaches '%s'. The monitor closed, "
                      "or the board reset, before the block finished. Take the run again." % (path, END))
    missing = [k for k in COMMON if k not in d]
    if missing:
        raise Invalid("the block in %s is missing %s. Either the log was edited or w11_common.h in "
                      "src/ is not the pack's." % (path, ", ".join(missing)))
    if d["sketch"] not in TESTS:
        raise Invalid("sketch = %r is not one of this pack's three sketches." % d["sketch"])
    return d


# ----------------------------------------------------------------------------- checking

def fnum(d, k):
    try:
        return float(d[k])
    except (KeyError, ValueError):
        return None


def check(d):
    """(refuse, notes, figures). `refuse` non-empty means VERDICT NOT VALID. `figures` maps each
    test name to ('ok', ns) or ('REFUSED', reason)."""
    spec = TESTS[d["sketch"]]
    refuse, notes, figures = [], [], {}

    cpu = fnum(d, "cpu_mhz")
    if cpu is None or cpu <= 0:
        refuse.append("cpu_mhz is not a positive number; no ns figure can be derived from cycles")
        return refuse, notes, figures
    floor_ns = 1000.0 / cpu
    if cpu != 240:
        notes.append("cpu_mhz = %g, not 240. Figures are still in ns, but say so on the sheet." % cpu)
    spread_max = fnum(d, "spread_max_pct")
    psram_bytes = fnum(d, "psram_bytes")

    for k in spec["extra"]:
        if k not in d:
            refuse.append("key %s missing: the sketch in src/ is not the pack's %s" % (k, d["sketch"]))

    for name, tier in spec["tests"].items():
        st = d.get(name + "_status")
        ns_s = d.get(name + "_ns_per_item")
        if st is None or ns_s is None:
            refuse.append("%s: status or ns_per_item line missing" % name)
            continue
        if st.startswith("REFUSED"):
            if ns_s != "n/a":
                refuse.append("%s is REFUSED by the sketch but still carries a figure (%s)" % (name, ns_s))
            figures[name] = ("REFUSED", st[len("REFUSED"):].lstrip(": "))
            continue
        if st != "ok":
            refuse.append("%s_status = %r is neither ok nor REFUSED" % (name, st))
            continue
        try:
            ns = float(ns_s)
        except ValueError:
            refuse.append("%s says ok but ns_per_item = %r is not a number" % (name, ns_s))
            continue
        # The sketch's own rules, re-applied here. Disagreement = the block is lying.
        if ns < floor_ns:
            refuse.append("%s = %.2f ns is below one CPU cycle (%.2f ns) yet marked ok" % (name, ns, floor_ns))
        sp = fnum(d, name + "_spread_pct")
        if sp is None:
            refuse.append("%s_spread_pct missing" % name)
        elif spread_max is not None and sp > spread_max:
            refuse.append("%s spread %.1f%% exceeds the sketch's own limit %g%% yet marked ok" % (name, sp, spread_max))
        if tier is not None:
            got = d.get(spec["tier_key"](name) + "_tier")
            if got != tier:
                refuse.append("%s claims %s but its buffer was in %s" % (name, tier, got))
            if tier == "PSRAM" and psram_bytes == 0:
                refuse.append("%s says ok with psram_bytes = 0: the tier check was bypassed" % name)
        if "derived" in spec:
            suffix, mult_key, scale = spec["derived"]
            dv = fnum(d, name + suffix)
            mult = fnum(d, mult_key) if mult_key else 1.0
            if dv is None or mult is None:
                refuse.append("%s%s missing" % (name, suffix))
            else:
                want = ns * mult * scale
                if abs(dv - want) > max(0.02 * want, 0.05):
                    refuse.append("%s%s = %g disagrees with ns_per_item x %g = %g" % (name, suffix, dv, mult * scale, want))
        figures[name] = ("ok", ns)

    for k in spec["balance"]:
        v = d.get(k)
        if v is None:
            # A tier whose tests were all refused before timing (no PSRAM, no free block)
            # never reaches its balance line. That is consistent, not missing.
            tag = k[:-len("_balance")] if k.endswith("_balance") else None
            owned = [n for n in spec["tests"] if tag and n.startswith(tag + "_")]
            if owned and all(figures.get(n, ("", ""))[0] == "REFUSED" for n in owned):
                continue
            refuse.append("%s missing" % k)
        elif not v.startswith("ok"):
            refuse.append("%s = %s: the sketch's own self-check failed; no figure in this block stands" % (k, v))

    if figures and all(v[0] == "REFUSED" for v in figures.values()):
        refuse.append("every test in this block was refused; there is no measurement here")

    if d.get("psram_found_api") == "1" and psram_bytes == 0:
        notes.append("psramFound() said 1 while the heap has 0 PSRAM bytes. The API is not the test.")
    if "pin_refused" in d:
        notes.append("pin_refused: " + d["pin_refused"])
    return refuse, notes, figures


# ----------------------------------------------------------------------------- output

def main(argv):
    if not argv:
        sys.exit("usage: python report.py <log-or-pattern> [--block N]")
    arg, block_no = argv[0], None
    if "--block" in argv:
        try:
            block_no = int(argv[argv.index("--block") + 1])
        except (IndexError, ValueError):
            sys.exit("--block needs a number")
    try:
        path = resolve_path(arg)
        d = parse(path, block_no)
    except Invalid as e:
        print("REFUSED  %s" % e)
        print("VERDICT  NOT VALID")
        return 2

    spec = TESTS[d["sketch"]]
    print("file     %s" % path)
    print("sketch   %s   (figures are ns per %s, median of %s passes)" % (d["sketch"], spec["per"], d.get("repeats")))
    for k in COMMON[1:] + spec["extra"] + spec["balance"]:
        if k in d:
            print("  %-24s %s" % (k, d[k]))
    refuse, notes, figures = check(d)
    print("FIGURES")
    for name in spec["tests"]:
        if name not in figures:
            print("  %-18s (missing)" % name)
            continue
        st, v = figures[name]
        if st == "ok":
            line = "  %-18s ok      %9.2f ns/%s" % (name, v, spec["per"])
            if "derived" in spec:
                line += "   %s %s" % (d.get(name + spec["derived"][0]), spec["derived"][0].strip("_").replace("_", " "))
            print(line)
        else:
            print("  %-18s REFUSED %s" % (name, v))
    for n in notes:
        print("NOTE     %s" % n)
    for r in refuse:
        print("REFUSED  %s" % r)
    if refuse:
        print("VERDICT  NOT VALID")
        return 2
    print("VERDICT  valid  -  copy only the figures marked ok; a REFUSED figure is not a measurement")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
