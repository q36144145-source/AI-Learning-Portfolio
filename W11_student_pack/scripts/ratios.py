# ratios.py  -  put the three runs side by side and work out the ratios the record sheet asks for.
# Usage:  python ratios.py run1-hierarchy.txt run2-dense.txt run3-sparse.txt
#         (any subset works: python ratios.py run1-hierarchy.txt)
#
# Every file goes through report.py's checks first. A ratio is printed only when BOTH of its
# sides are figures the sketch marked ok; otherwise the cell says n/a and names the side that
# was refused. No ratio is ever formed against a refused or missing figure, and no figure is
# ever divided by zero - an "ok" figure is at least one CPU cycle by construction.
# Standard library only.
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report import parse, check, resolve_path, Invalid, fnum   # noqa: E402


def load(arg):
    path = resolve_path(arg)
    d = parse(path)
    refuse, _, figures = check(d)
    if refuse:
        raise Invalid("%s is not a valid run:\n    - %s\n  Run report.py on it for the full story."
                      % (path, "\n    - ".join(refuse)))
    return d, figures


def fig(figures, name):
    """('ok', value) or ('REFUSED', reason) or ('missing', '')."""
    return figures.get(name, ("missing", ""))


def ratio(figures, num, den, label):
    a, b = fig(figures, num), fig(figures, den)
    if a[0] != "ok" or b[0] != "ok":
        bad = num if a[0] != "ok" else den
        print("  %-44s n/a   (%s %s)" % (label, bad, fig(figures, bad)[0]))
        return
    print("  %-44s %8.2f" % (label, a[1] / b[1]))


def hierarchy(d, f):
    print("RUN 1  %s  -  ns per word, median" % d["sketch"])
    print("  %-14s %10s %10s %10s" % ("tier/size", "seq", "rand", "rand/seq"))
    for tag in ["sram", "psram128k", "psram1m", "flash128k", "flash1m"]:
        s, r = fig(f, tag + "_seq"), fig(f, tag + "_rand")
        sv = "%10.2f" % s[1] if s[0] == "ok" else "%10s" % s[0]
        rv = "%10.2f" % r[1] if r[0] == "ok" else "%10s" % r[0]
        q = "%10.2f" % (r[1] / s[1]) if (s[0] == "ok" and r[0] == "ok") else "%10s" % "n/a"
        print("  %-14s %s %s %s" % (tag, sv, rv, q))
    c = fig(f, "control_noload")
    print("  %-14s %10s %10s   (loop + index arithmetic, no memory read; inside every rand figure)"
          % ("control", "", "%.2f" % c[1] if c[0] == "ok" else c[0]))
    print("RATIOS against SRAM (same pattern)")
    for tag in ["psram128k", "psram1m", "flash128k", "flash1m"]:
        ratio(f, tag + "_seq", "sram_seq", "%s seq / sram seq" % tag)
        ratio(f, tag + "_rand", "sram_rand", "%s rand / sram rand" % tag)
    print("THE CACHE: same tier, 1 MB against 128 KB (random)")
    ratio(f, "psram1m_rand", "psram128k_rand", "psram 1M rand / psram 128K rand")
    ratio(f, "flash1m_rand", "flash128k_rand", "flash 1M rand / flash 128K rand")
    print("  malloc(2048) landed in %s, malloc(65536) landed in %s, psram_bytes = %s"
          % (d.get("malloc_2k_tier"), d.get("malloc_64k_tier"), d.get("psram_bytes")))


def dense(d, f):
    print("RUN 2  %s  -  us per inference (%s MACs), median" % (d["sketch"], d.get("macs_per_inference")))
    print("  %-10s %12s %12s %12s" % ("", "SRAM", "PSRAM", "FLASH"))
    for typ in ["f32", "s8"]:
        cells = []
        for tier in ["sram", "psram", "flash"]:
            name = "%s_%s" % (typ, tier)
            v = fig(f, name)
            cells.append("%12s" % (d.get(name + "_us_per_inference") if v[0] == "ok" else v[0]))
        print("  %-10s %s" % (typ, " ".join(cells)))
    print("  weights: f32 %s bytes, s8 %s bytes + %s bytes of scale   (computed from the shape)"
          % (d.get("weights_bytes_f32"), d.get("weights_bytes_s8"), d.get("scale_bytes_s8")))
    print("WIDTH: float32 / int8, same tier")
    for tier in ["sram", "psram", "flash"]:
        ratio(f, "f32_" + tier, "s8_" + tier, "f32 %s / s8 %s" % (tier, tier))
    print("TIER: against SRAM, same width")
    for typ in ["f32", "s8"]:
        for tier in ["psram", "flash"]:
            ratio(f, "%s_%s" % (typ, tier), "%s_sram" % typ, "%s %s / %s sram" % (typ, tier, typ))
    print("CROSS: the slow memory with the narrow weights against the fast memory with the wide ones")
    ratio(f, "f32_sram", "s8_flash", "f32 sram / s8 flash")
    print("ERROR (measured on this input): rel rms %s %%, max abs %s, half-step bound per weight %s"
          % (d.get("quant_rel_rms_err_pct"), d.get("quant_max_abs_err"), d.get("quant_half_step_w")))


def sparse(d, f, dense_d=None, dense_f=None):
    print("RUN 3  %s  -  us per token (%s rows x %s bytes = %s bytes touched), median"
          % (d["sketch"], d.get("rows_per_token"), d.get("row_bytes"), d.get("bytes_per_token")))
    for name in ["flash1m_lookup", "psram1m_lookup", "flash16k_lookup", "psram16k_lookup", "sram16k_lookup"]:
        v = fig(f, name)
        print("  %-16s %10s   table %s bytes" % (name, d.get(name + "_us_per_token") if v[0] == "ok" else v[0],
                                                   d.get(name + "_table_bytes")))
    print("  sram_holds_table = %s" % d.get("sram_holds_table"))
    print("TIER, 1 MB table: flash / psram")
    ratio(f, "flash1m_lookup", "psram1m_lookup", "flash 1M lookup / psram 1M lookup")
    print("THE CACHE: 1 MB table / 16 KB table, same tier (the 16 KB one is a cache figure, not a memory figure)")
    ratio(f, "flash1m_lookup", "flash16k_lookup", "flash 1M / flash 16K")
    ratio(f, "psram1m_lookup", "psram16k_lookup", "psram 1M / psram 16K")
    tb = fnum(d, "flash1m_lookup_table_bytes")
    bt = fnum(d, "bytes_per_token")
    if tb and bt:
        print("  fraction of the table touched per token: %d / %d = %.4f %%" % (bt, tb, 100.0 * bt / tb))
    if dense_d is not None:
        s8 = fig(dense_f, "s8_sram")
        fl = fig(f, "flash1m_lookup")
        if s8[0] == "ok" and fl[0] == "ok":
            us_inf = fnum(dense_d, "s8_sram_us_per_inference")
            us_tok = fnum(d, "flash1m_lookup_us_per_token")
            if us_inf and us_tok and us_tok > 0:
                print("ACROSS RUNS: one int8 dense inference from SRAM (run 2) / one 6-row flash lookup (run 3)")
                print("  %-44s %8.2f   (%s us / %s us)" % ("s8 sram inference / flash1m lookup", us_inf / us_tok, us_inf, us_tok))
        else:
            print("ACROSS RUNS: n/a (%s)" % ("s8_sram " + s8[0] if s8[0] != "ok" else "flash1m_lookup " + fl[0]))


def main(argv):
    if not argv:
        sys.exit("usage: python ratios.py <run1> [run2] [run3]")
    import os
    for a in argv:
        if not os.path.isfile(a):
            sys.exit("file not found: %s -- did you move the log into w11/ and rename it?" % a)
    runs = {}
    try:
        for a in argv:
            d, f = load(a)
            if d["sketch"] in runs:
                raise Invalid("two files carry sketch %s; name one run per sketch" % d["sketch"])
            runs[d["sketch"]] = (d, f)
    except Invalid as e:
        print("REFUSED  %s" % e)
        return 2
    if "1_hierarchy" in runs:
        hierarchy(*runs["1_hierarchy"])
    if "2_dense" in runs:
        dense(*runs["2_dense"])
    if "3_sparse" in runs:
        dd, df = runs.get("2_dense", (None, None))
        sparse(runs["3_sparse"][0], runs["3_sparse"][1], dd, df)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
