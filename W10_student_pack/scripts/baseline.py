# baseline.py  -  the classical baseline: your three thresholds, applied to the test windows,
#                 scored with the same confusion matrix the model will get.
#
# Usage:  python baseline.py w10_out --tap 2.5 --shake 0.15 --tilt 30
#
# The three numbers are YOURS, read off w10_out/feature_table.txt (training session only).
# The rule is the one in sketch_c_classify.cpp, in the same order:
#     amag_max  > tap    -> tap
#     amag_std  > shake  -> shake
#     gmag_mean > tilt   -> tilt
#     otherwise          -> still
# Writes w10_out/cm_rule.json for compare.py. Refuses a threshold that is missing or <= 0,
# and refuses to score a folder that has no test set. If the folder came from selftest.py
# (manifest says synthetic), every matrix is followed by a NOTE saying so, and the flag is
# written into cm_rule.json so that compare.py says so too - a believable number from data
# that is not data is the one output this pack must never produce silently.
import argparse
import json
import os
import sys

import numpy as np

from w10lib import CLASSES, FEAT_NAMES, Refused, confusion, format_cm, refused, write_cm


def read_features(path):
    if not os.path.isfile(path):
        raise Refused("%s does not exist. Run windows.py first, with --test." % path)
    ids, y, X = [], [], []
    with open(path, encoding="utf-8") as f:
        head = f.readline().strip().split(",")
        if head[4:] != FEAT_NAMES:
            raise Refused("%s has columns %s; this pack expects %s. Re-run windows.py from this pack." % (path, head[4:], FEAT_NAMES))
        for line in f:
            p = line.strip().split(",")
            if len(p) != 4 + len(FEAT_NAMES):
                continue
            ids.append(int(p[0])); y.append(int(p[2])); X.append([float(v) for v in p[4:]])
    if not ids:
        raise Refused("%s has no rows." % path)
    return np.array(ids), np.array(y), np.array(X)


def rule(X, tap, shake, tilt):
    out = np.zeros(len(X), dtype=int)
    for i, f in enumerate(X):
        if f[2] > tap:      out[i] = 3
        elif f[1] > shake:  out[i] = 2
        elif f[4] > tilt:   out[i] = 1
        else:               out[i] = 0
    return out


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("out", help="the folder windows.py wrote")
    ap.add_argument("--tap", type=float, help="T_TAP_G: amag_max above this is a tap (g)")
    ap.add_argument("--shake", type=float, help="T_SHAKE_G: amag_std above this is a shake (g)")
    ap.add_argument("--tilt", type=float, help="T_TILT_DPS: gmag_mean above this is a tilt (deg/s)")
    a = ap.parse_args(argv)
    try:
        for name, v in (("--tap", a.tap), ("--shake", a.shake), ("--tilt", a.tilt)):
            if v is None:
                raise Refused("%s not given. All three thresholds come off your feature_table.txt; "
                              "this script does not have defaults, because a default would be a "
                              "number you did not choose." % name)
            if not (v > 0):
                raise Refused("%s = %r. A threshold at or below zero fires on every window; that is "
                              "not a rule, it is a constant answer." % (name, v))
        idtr, ytr, Xtr = read_features(os.path.join(a.out, "features_train.csv"))
        idte, yte, Xte = read_features(os.path.join(a.out, "features_test.csv"))
    except Refused as e:
        return refused(e)
    synthetic = False
    mpath = os.path.join(a.out, "manifest.json")
    if os.path.isfile(mpath):
        with open(mpath, encoding="utf-8") as f:
            synthetic = bool(json.load(f).get("synthetic"))

    cm_tr = confusion(ytr, rule(Xtr, a.tap, a.shake, a.tilt))
    cm_te = confusion(yte, rule(Xte, a.tap, a.shake, a.tilt))
    print("RULE   tap: amag_max > %.3f g   shake: amag_std > %.3f g   tilt: gmag_mean > %.1f dps" % (a.tap, a.shake, a.tilt))
    print()
    print(format_cm(cm_tr, "TRAINING session (the one you chose the thresholds from - expect this to look good)"))
    print()
    print(format_cm(cm_te, "TEST session (the one that counts)"))
    write_cm(os.path.join(a.out, "cm_rule.json"), "rule", cm_te, idte,
             {"thresholds": {"tap": a.tap, "shake": a.shake, "tilt": a.tilt}, "synthetic": synthetic})
    print()
    print("WROTE  %s/cm_rule.json  (the TEST matrix; compare.py reads it)" % a.out)
    print("       Put the same three numbers into sketch_c_classify.cpp: T_TAP_G, T_SHAKE_G, T_TILT_DPS.")
    if synthetic:
        print("NOTE   source=SYNTHETIC: these matrices score selftest data, not a session from your board. Not a result.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
