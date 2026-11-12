# compare.py  -  the rule and the model, side by side, on the SAME test windows.
#
# Usage:  python compare.py w10_out/cm_rule.json w10_out/cm_model.json
#         python compare.py w10_out/cm_rule.json w10_out/cm_model.json \
#                 --manual "7,0,1,0;0,8,0,0;0,0,8,0;1,0,0,7" --manual-name edge-impulse
#
# The two JSON files were written by baseline.py and train.py and carry the ids of the test
# windows they were scored on. If the ids differ, the matrices are not comparable and this
# script says so instead of printing a difference. A --manual matrix (copied by hand from
# the Edge Impulse Model testing page, rows = true class in still,tilt,shake,tap order)
# cannot be checked that way; it is printed with that caveat. If either JSON carries the
# synthetic flag (selftest data), the comparison ends with a NOTE saying so.
import argparse
import sys

import numpy as np

from w10lib import CLASSES, Refused, format_cm, metrics, read_cm, refused, worst_cell


def pct(v):
    return "  n/a " if v is None else "%5.1f%%" % (100 * v)


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("rule_json")
    ap.add_argument("model_json")
    ap.add_argument("--manual", help="a third 4x4 matrix typed in: 'r1;r2;r3;r4', each row 4 comma-separated counts")
    ap.add_argument("--manual-name", default="manual")
    a = ap.parse_args(argv)
    try:
        n1, cm1, ids1, d1 = read_cm(a.rule_json)
        n2, cm2, ids2, d2 = read_cm(a.model_json)
        if sorted(ids1) != sorted(ids2):
            raise Refused("%s was scored on windows %s and %s on %s. These are not the same test "
                          "windows, so the two matrices cannot be compared. Re-run baseline.py and "
                          "train.py on the same w10_out folder." % (a.rule_json, sorted(ids1), a.model_json, sorted(ids2)))
        if cm1.sum() != cm2.sum():
            raise Refused("the two matrices count a different number of windows (%d vs %d)." % (cm1.sum(), cm2.sum()))
        cms = [(n1, cm1, True), (n2, cm2, True)]
        if a.manual:
            try:
                rows = [[int(x) for x in r.split(",")] for r in a.manual.split(";")]
                cm3 = np.array(rows, dtype=int)
                assert cm3.shape == (4, 4)
            except Exception:
                raise Refused("--manual must be four rows of four integers, e.g. \"7,0,1,0;0,8,0,0;0,0,8,0;1,0,0,7\".")
            if cm3.sum() != cm1.sum():
                raise Refused("the manual matrix counts %d windows but the test set has %d. Did you upload "
                              "exactly the ei_export/testing folder as the test set, and nothing else?"
                              % (cm3.sum(), cm1.sum()))
            cms.append((a.manual_name, cm3, False))
    except Refused as e:
        return refused(e)

    for name, cm, _ in cms:
        print(format_cm(cm, "== %s ==" % name))
        print()
    ms = [metrics(cm) for _, cm, _ in cms]
    head = "%-12s" % "" + "".join("%14s" % n for n, _, _ in cms)
    print("PER-CLASS RECALL   (of the windows where you did X, how many did it call X)")
    print(head)
    for i, c in enumerate(CLASSES):
        print("%-12s" % c + "".join("%14s" % pct(m["recall"][i]) for m in ms))
    print()
    print("PER-CLASS PRECISION (of the windows it called X, how many were X)")
    print(head)
    for i, c in enumerate(CLASSES):
        print("%-12s" % c + "".join("%14s" % pct(m["precision"][i]) for m in ms))
    print()
    print("%-12s" % "accuracy" + "".join("%14s" % pct(m["accuracy"]) for m in ms))
    print("%-12s" % "majority" + "".join("%14s" % pct(m["majority_accuracy"]) for m in ms))
    print()
    # what the model bought, class by class, against the rule
    m1, m2 = ms[0], ms[1]
    gains = []
    for i, c in enumerate(CLASSES):
        r1, r2 = m1["recall"][i], m2["recall"][i]
        if r1 is None or r2 is None:
            continue
        gains.append((100 * (r2 - r1), c))
    if gains:
        up = [g for g in gains if g[0] > 0]; down = [g for g in gains if g[0] < 0]
        print("WHAT THE MODEL CHANGED, recall, model minus rule, on these %d test windows:" % cm1.sum())
        for g, c in gains:
            print("   %-8s %+6.1f points" % (c, g))
        if not up and not down:
            print("   nothing. On this test set the rule and the model agree on every class.")
        print("   step size on this test set is 1/n per class - a difference smaller than one window is not a difference.")
    w1, w2 = worst_cell(cm1), worst_cell(cm2)
    print()
    print("LARGEST ERROR   rule: %s   model: %s" % (
        "none" if not w1 else "did %s, said %s x%d" % w1, "none" if not w2 else "did %s, said %s x%d" % w2))
    if len(cms) > 2:
        print()
        print("NOTE   '%s' was typed in by hand; its window identity was not verified by this script." % cms[2][0])
    if d1.get("synthetic") or d2.get("synthetic"):
        print()
        print("NOTE   source=SYNTHETIC: at least one of these matrices was scored on selftest data, not on a session from your board. Not a result.")
    print()
    print("The cost side - bytes, microseconds, RAM, Flash - is not in these files. It is on the board. Sheet section 6.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
