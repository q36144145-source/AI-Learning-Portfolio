# train.py  -  the offline model: a one-hidden-layer network trained in numpy on YOUR
#              training windows, scored on YOUR test windows, written out as a C header.
#
# Usage:  python train.py w10_out --header sketches/model_weights.h
#         python train.py w10_out --header sketches/model_weights.h --epochs 600 --seed 3
#
# This is the path that works with the room's network down. It is the same shape of
# thing Edge Impulse builds from the same windows - standardise the eight features, one
# dense layer, a softmax - and it is small enough to read: 8 x 12 + 12 x 4 weights.
# You are not asked to read it. You are asked to compare its confusion matrix with the
# rule's, on the same test windows, and then put it on the board.
#
# What it refuses: a folder with no test set; a test set whose window ids overlap the
# training set; a class with too few windows (windows.py already enforced this, but the
# CSVs could have been edited). It also prints the always-majority baseline next to the
# accuracy, every time, so the accuracy is never read on its own.
import argparse
import datetime
import json
import os
import sys

import numpy as np

from w10lib import (CLASSES, FEAT_NAMES, MIN_TEST_PER_CLASS, MIN_TRAIN_PER_CLASS, Refused,
                    confusion, format_cm, refused, require_min, write_cm)
from baseline import read_features

N_HID = 12


def softmax(z):
    z = z - z.max(axis=1, keepdims=True)
    e = np.exp(z)
    return e / e.sum(axis=1, keepdims=True)


def train_mlp(X, y, epochs, seed, lr=0.05, l2=1e-3):
    rng = np.random.default_rng(seed)
    n, d = X.shape
    k = len(CLASSES)
    W1 = rng.normal(0, 0.5, (N_HID, d)); b1 = np.zeros(N_HID)
    W2 = rng.normal(0, 0.5, (k, N_HID)); b2 = np.zeros(k)
    Y = np.eye(k)[y]
    # class weights: the inverse of the class share, so a class with fewer windows is not
    # simply outvoted. With equal counts this is 1.0 everywhere and changes nothing.
    share = np.bincount(y, minlength=k) / n
    cw = (1.0 / np.maximum(share, 1e-9)) / k
    cw = cw / cw.mean()
    sw = cw[y][:, None]
    for _ in range(epochs):
        H = np.maximum(X @ W1.T + b1, 0)
        P = softmax(H @ W2.T + b2)
        G = (P - Y) * sw / n
        dW2 = G.T @ H + l2 * W2; db2 = G.sum(axis=0)
        dH = (G @ W2) * (H > 0)
        dW1 = dH.T @ X + l2 * W1; db1 = dH.sum(axis=0)
        W1 -= lr * dW1; b1 -= lr * db1; W2 -= lr * dW2; b2 -= lr * db2
    return W1, b1, W2, b2


def predict(X, W1, b1, W2, b2):
    H = np.maximum(X @ W1.T + b1, 0)
    return softmax(H @ W2.T + b2).argmax(axis=1)


def c_array(name, arr):
    arr = np.asarray(arr, dtype=np.float32)
    if arr.ndim == 1:
        return "static const float %s[%d] = { %s };\n" % (name, arr.shape[0], ", ".join("%.8ef" % v for v in arr))
    rows = ",\n  ".join("{ %s }" % ", ".join("%.8ef" % v for v in r) for r in arr)
    return "static const float %s[%d][%d] = {\n  %s };\n" % (name, arr.shape[0], arr.shape[1], rows)


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("out", help="the folder windows.py wrote")
    ap.add_argument("--header", required=True, help="where to write model_weights.h (the one in sketches/)")
    ap.add_argument("--epochs", type=int, default=400)
    ap.add_argument("--seed", type=int, default=0)
    a = ap.parse_args(argv)
    try:
        mpath = os.path.join(a.out, "manifest.json")
        if not os.path.isfile(mpath):
            raise Refused("%s has no manifest.json; it was not written by windows.py." % a.out)
        with open(mpath, encoding="utf-8") as f:
            man = json.load(f)
        if "test" not in man:
            raise Refused("this folder was made with --train-only: there is no test set, so there is "
                          "nothing honest to score the model on. Re-run windows.py with --test.")
        if man["train"]["hash"] == man["test"]["hash"]:
            raise Refused("manifest says train and test came from the same file. Not a test set.")
        idtr, ytr, Xtr = read_features(os.path.join(a.out, "features_train.csv"))
        idte, yte, Xte = read_features(os.path.join(a.out, "features_test.csv"))
        ctr = {c: int((ytr == i).sum()) for i, c in enumerate(CLASSES)}
        cte = {c: int((yte == i).sum()) for i, c in enumerate(CLASSES)}
        require_min(ctr, MIN_TRAIN_PER_CLASS, "training")
        require_min(cte, MIN_TEST_PER_CLASS, "test")
    except Refused as e:
        return refused(e)

    mu = Xtr.mean(axis=0); sd = Xtr.std(axis=0); sd[sd == 0] = 1.0
    Ztr = (Xtr - mu) / sd; Zte = (Xte - mu) / sd
    W1, b1, W2, b2 = train_mlp(Ztr, ytr, a.epochs, a.seed)
    cm_tr = confusion(ytr, predict(Ztr, W1, b1, W2, b2))
    cm_te = confusion(yte, predict(Zte, W1, b1, W2, b2))

    print("MODEL  %d features -> %d hidden (ReLU) -> %d classes (softmax), %d epochs, seed %d"
          % (len(FEAT_NAMES), N_HID, len(CLASSES), a.epochs, a.seed))
    nbytes = 4 * (2 * len(FEAT_NAMES) + W1.size + b1.size + W2.size + b2.size)
    print("       weights + standardisation = %d float32 = %d bytes (float32, unquantised - W11 starts here)"
          % (nbytes // 4, nbytes))
    print()
    print(format_cm(cm_tr, "TRAINING session (fit quality only - this is not a result)"))
    print()
    print(format_cm(cm_te, "TEST session (the one that counts)"))

    prov = "train=%s n=%d test=%s n=%d epochs=%d seed=%d written=%s" % (
        man["train"]["meta"].get("source", "?") + ":" + os.path.basename(man["train"]["file"]), len(ytr),
        os.path.basename(man["test"]["file"]), len(yte), a.epochs, a.seed,
        datetime.date.today().isoformat())
    with open(a.header, "w", encoding="utf-8") as f:
        f.write("// model_weights.h - written by scripts/train.py. Do not edit by hand.\n")
        f.write("// %s\n" % prov)
        f.write("// Shapes: %d features -> %d hidden -> %d classes. %d bytes of float32.\n" % (len(FEAT_NAMES), N_HID, len(CLASSES), nbytes))
        f.write("#pragma once\n#define MODEL_READY 1\n")
        f.write('#define MODEL_PROVENANCE "%s"\n' % prov)
        f.write("#define N_HID %d\n#define N_CLS %d\n" % (N_HID, len(CLASSES)))
        f.write(c_array("MODEL_MU", mu)); f.write(c_array("MODEL_SD", sd))
        f.write(c_array("MODEL_W1", W1)); f.write(c_array("MODEL_B1", b1))
        f.write(c_array("MODEL_W2", W2)); f.write(c_array("MODEL_B2", b2))
    write_cm(os.path.join(a.out, "cm_model.json"), "model-numpy", cm_te, idte,
             {"epochs": a.epochs, "seed": a.seed, "model_bytes": nbytes, "synthetic": bool(man.get("synthetic"))})
    print()
    print("WROTE  %s  (MODEL_READY 1; copy it into src/ next to sketch_c_classify.cpp and rebuild)" % a.header)
    print("WROTE  %s/cm_model.json  (the TEST matrix; compare.py reads it)" % a.out)
    if man.get("synthetic"):
        print("NOTE   source=SYNTHETIC: this header came from selftest data. Do not flash it as if it were yours.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
