# windows.py  -  turn two session logs into a training set and a test set, and REFUSE
#                to make a test set out of the same session as the training set.
#
# Usage:  python windows.py sessionA.txt --test sessionB.txt --out w10_out
#         python windows.py sessionA.txt --test neighbour_sessionA.txt --out w10_out
#         python windows.py sessionA.txt --train-only --out w10_out      (no test set: see below)
#
# Writes into --out:
#   features_train.csv, features_test.csv   id,label,y,f0..f7     (what train.py and baseline.py read)
#   feature_table.txt                       per class, per feature: min / median / max.
#                                           FROM THE TRAINING SESSION ONLY. Your three rule
#                                           thresholds come off this table.
#   manifest.json                           which files, which windows, what was dropped and why
#   ei_export/training/<label>.<stem>.wNN.json   one file per window, Edge Impulse format
#   ei_export/testing/<label>.<stem>.wNN.json    upload each folder into the matching category
#
# Exit 0 = the sets are usable. Exit 2 = refused, reason printed, nothing partial written.
import argparse
import json
import os
import shutil
import sys

import numpy as np

from w10lib import (CLASSES, FEAT_NAMES, MIN_TEST_PER_CLASS, MIN_TRAIN_PER_CLASS, Refused,
                    class_counts, ei_json, feature_matrix, feature_table, parse_log, refused,
                    require_min)


def write_features(path, windows, X, y, ids, session):
    with open(path, "w", encoding="utf-8") as f:
        f.write("id,label,y,session," + ",".join(FEAT_NAMES) + "\n")
        for i in range(len(ids)):
            f.write("%d,%s,%d,%s," % (ids[i], CLASSES[y[i]], y[i], session) + ",".join("%.6f" % v for v in X[i]) + "\n")


def export_ei(folder, log, windows, accel_only):
    os.makedirs(folder, exist_ok=True)
    for w in windows:
        name = "%s.%s.w%02d.json" % (w["label"], log["stem"].replace(".", "_"), w["id"])
        with open(os.path.join(folder, name), "w", encoding="utf-8") as f:
            json.dump(ei_json(w, log["stem"], accel_only), f)
    return len(windows)


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("train_log")
    ap.add_argument("--test", help="a DIFFERENT session's log: yours, recorded after putting the board down, or a neighbour's")
    ap.add_argument("--train-only", action="store_true", help="export the training set with no test set at all")
    ap.add_argument("--out", default="w10_out")
    ap.add_argument("--selftest", action="store_true", help="accept source=SYNTHETIC logs (selftest.py only)")
    ap.add_argument("--ei-accel-only", action="store_true", help="Edge Impulse export with the three accelerometer axes only")
    a = ap.parse_args(argv)

    try:
        if not a.test and not a.train_only:
            raise Refused("no --test given. This script will not split one session's windows at random "
                          "into 'train' and 'test': windows two seconds apart from the same hand in the "
                          "same sitting are not independent, and a test score on them measures how "
                          "consistent you were for three minutes, not whether the model generalises. "
                          "Record a second session (put the board down first) and pass it as --test, "
                          "or use a neighbour's session - or say --train-only and accept that you "
                          "have no test set.")
        if a.test and a.train_only:
            raise Refused("--test and --train-only together make no sense.")
        tr = parse_log(a.train_log, allow_synthetic=a.selftest)
        te = None
        if a.test:
            te = parse_log(a.test, allow_synthetic=a.selftest)
            if te["hash"] == tr["hash"]:
                raise Refused("%s and %s are the same file (same content). The test set would be the "
                              "training set, and every score would be 100%% and meaningless."
                              % (a.train_log, a.test))
            shared = {w["rowhash"] for w in tr["windows"]} & {w["rowhash"] for w in te["windows"]}
            if shared:
                raise Refused("%d window(s) appear in BOTH logs with identical samples. A test set that "
                              "overlaps the training set is not a test set." % len(shared))

        ctr = class_counts(tr["windows"])
        require_min(ctr, MIN_TRAIN_PER_CLASS, "training")
        if te:
            cte = class_counts(te["windows"])
            require_min(cte, MIN_TEST_PER_CLASS, "test")
    except Refused as e:
        return refused(e)

    # ---- everything checks out: write the outputs ----------------------------------
    if os.path.isdir(a.out):
        shutil.rmtree(a.out)
    os.makedirs(a.out)
    Xtr, ytr, idtr = feature_matrix(tr["windows"])
    write_features(os.path.join(a.out, "features_train.csv"), tr["windows"], Xtr, ytr, idtr, tr["stem"])
    table = feature_table(Xtr, ytr)
    with open(os.path.join(a.out, "feature_table.txt"), "w", encoding="utf-8") as f:
        f.write("# feature table - TRAINING SESSION ONLY (%s). Choose T_TAP_G from amag_max,\n" % tr["stem"])
        f.write("# T_SHAKE_G from amag_std, T_TILT_DPS from gmag_mean. Do not look at the test session.\n")
        f.write(table + "\n")
    n_ei_tr = export_ei(os.path.join(a.out, "ei_export", "training"), tr, tr["windows"], a.ei_accel_only)
    manifest = {
        "train": {"file": tr["path"], "hash": tr["hash"], "windows": idtr, "counts": ctr,
                  "dropped": tr["dropped"], "meta": tr["meta"]},
        "classes": CLASSES, "features": FEAT_NAMES,
        "synthetic": tr["meta"].get("source", "").upper() == "SYNTHETIC",
    }
    n_ei_te = 0
    if te:
        Xte, yte, idte = feature_matrix(te["windows"])
        write_features(os.path.join(a.out, "features_test.csv"), te["windows"], Xte, yte, idte, te["stem"])
        n_ei_te = export_ei(os.path.join(a.out, "ei_export", "testing"), te, te["windows"], a.ei_accel_only)
        manifest["test"] = {"file": te["path"], "hash": te["hash"], "windows": idte, "counts": cte,
                            "dropped": te["dropped"], "meta": te["meta"]}
    with open(os.path.join(a.out, "manifest.json"), "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=1)

    # ---- say what happened ----------------------------------------------------------
    print("TRAIN  %s  %d windows kept, %d dropped" % (tr["path"], len(tr["windows"]), len(tr["dropped"])))
    for wid, lab, why in tr["dropped"]:
        print("         dropped window %d (%s): %s" % (wid, lab, why))
    print("         per class: " + "  ".join("%s=%d" % (k, ctr[k]) for k in CLASSES))
    if te:
        print("TEST   %s  %d windows kept, %d dropped" % (te["path"], len(te["windows"]), len(te["dropped"])))
        for wid, lab, why in te["dropped"]:
            print("         dropped window %d (%s): %s" % (wid, lab, why))
        print("         per class: " + "  ".join("%s=%d" % (k, cte[k]) for k in CLASSES))
        tot = sum(cte.values()); big = max(cte, key=cte.get)
        print("         a model that always says '%s' would score %.1f%% on this test set"
              % (big, 100.0 * cte[big] / tot))
    else:
        print("TEST   none (--train-only). No confusion matrix can be produced from this folder.")
    print()
    print(table)
    print()
    print("WROTE  %s/features_train.csv%s, feature_table.txt, manifest.json" % (a.out, ", features_test.csv" if te else ""))
    print("WROTE  %s/ei_export/training (%d files)%s" % (a.out, n_ei_tr, ", ei_export/testing (%d files)" % n_ei_te if te else ""))
    if manifest["synthetic"]:
        print("NOTE   source=SYNTHETIC: these outputs are a self-test of the pipeline, not data.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
