# W10 Student Pack — TinyML I: Data, Labelling & the Training Pipeline

**Embedded Systems (BME 3301) · Week 10 · Thursday 12 Nov, 09:10–12:00 · 醫技醫工館 5776**

Everything you run in class today is in this folder. You do not have to type any of it from
the slides — the code the slides show is cut from these files by line number, so it is
character-for-character identical to what is projected.

**`PREFLIGHT.md` is required before you come to class.** About twenty-five minutes, needs
internet, needs the board and the IMU. It is longer than W07's because this week has two
things the room's network cannot be trusted with: the first build, and an account on a web
service. Both get done at home.

---

## What today is

In W08 you wrote a threshold and were asked where the number came from. Today a tool offers
to learn one for you. You will:

1. collect two sessions of labelled IMU windows **off your own board**, through the same
   bring-up you verified in W05;
2. choose three thresholds from your own feature table and score that **rule** on the
   session it never saw — a confusion matrix, before any model exists;
3. train a model — in Edge Impulse Studio if the network allows, and offline with `train.py`
   either way — and score it on **the same test windows**;
4. put **both** on the board, on the same captured window, timed the same honest way, and
   fill in a cost sheet with four kinds of number on it.

The number you leave with is not an accuracy. It is a per-class difference, in windows,
between a rule you can explain and a model you cannot — next to what the model cost in bytes
and microseconds. **No accuracy appears on any slide**, and the model is not assumed to win.

## What is in here

```
platformio.ini              the same six lines as every week. No lib_deps: the model is a header
sketches/
  w10_imu.h                 W05's bring-up (five datasheet blanks, same halt= refusals), the
                            timer-paced 2 s window, and the eight features. Included by both sketches
  model_weights.h           PLACEHOLDER (MODEL_READY 0). train.py overwrites it with your model
  sketch_a_collect.cpp      one session: 32 prompted windows, 8 per class, printed after capture
  sketch_c_classify.cpp     the rule and the model on the board, 20 live windows, three micros() clocks
scripts/
  w10lib.py                 the log format, the eight features, the confusion matrix, the refusals
  windows.py                two logs -> training set + test set, feature table, Edge Impulse export.
                            REFUSES to split one session at random
  baseline.py               your three thresholds, scored on the test windows -> cm_rule.json
  train.py                  a one-hidden-layer model in numpy, scored on the same test windows,
                            written out as model_weights.h -> cm_model.json
  compare.py                rule and model side by side; refuses if the test windows differ
  selftest.py               generates SYNTHETIC logs and proves every refusal above fires
EDGE_IMPULSE.md             the L1 path, step by step, and what to do when the service is down
LAB_GUIDE.md                run by run, at your own pace. Work from this in class, not from the slides
RECORD_SHEET.md             fill this in as you go. It is the deliverable
AI_USAGE_template.md        the two entries you owe in AI_USAGE.md
PREFLIGHT.md                do this before class
```

`w10lib.py`, `windows.py`, `baseline.py`, `train.py` and `compare.py` need **numpy** and
nothing else. You installed numpy in W03.

## The four classes, and why they are these four

`still` · `tilt` · `shake` · `tap`. Four things you can do at a desk with a board on a
one-metre USB lead — moving the **breadboard the GY-521 sits in**, which is the part that senses. The syllabus's walk / run / fall assumed a wearable board, and the
board you have is not one. None of the four is a clinical event, and the deck says so on
slide 48; what they are is four classes a hand-written rule can mostly separate and a model
has to beat on the one it cannot.

## The numbers that are given to you, and why

Everything else on the record sheet is yours. These come from the linker on our machine
(5 Oct 2026, `espressif32@7.1.3`, arduino-esp32 2.0.17, `esp32-s3-devkitc-1`):

| Sketch | Flash | RAM |
|---|---|---|
| `sketch_a_collect.cpp` | 292,649 | 23,648 |
| `sketch_c_classify.cpp`, placeholder header | 295,069 | 23,700 |
| `sketch_c_classify.cpp`, a trained header | 295,133 | 23,700 |

Out of 3,342,336 bytes Flash and 327,680 bytes RAM. **RAM should match yours to the byte.**
Flash moves only with the length of the provenance string `train.py` writes into the
header — the 64-byte difference in the last two rows is exactly that string (26 and 92 bytes
once the terminating zero and 4-byte alignment are counted). It does **not** move with your
project path: on 14 Oct the same sketch was built in two unrelated folders and came out
identical to the byte. If yours differ by more than a few hundred bytes, that is worth a
sentence, not a shrug.

`train.py` also prints the model's size: 176 float32 values = **704 bytes**. That figure is
fixed by the shape (8 → 12 → 4) and does not depend on your data. It is the float32,
unquantised model; **W11 starts from it.**

## What has and has not been run

Both sketches build clean, and the three byte counts above were rebuilt from this exact pack
on 14 Oct and reproduced to the byte. **Neither has run on a board with an IMU attached** —
the dry run on Wed 11 Nov is the first time. The five Python scripts have been exercised end
to end on synthetic sessions by `selftest.py` (19 checks, all refusals included); they have
not seen a real log either. The Edge Impulse steps in `EDGE_IMPULSE.md` were written from the
documentation, not from a run. If anything in this pack behaves differently from what
`LAB_GUIDE.md` says it will, that is a finding and it goes in the course channel.

## Before Thursday

- [ ] `PREFLIGHT.md`, all five items — about twenty-five minutes
- [ ] Your five W05 datasheet values in `w10_imu.h`
- [ ] One throw-away session reaches `# done windows=32` — then delete it
- [ ] An Edge Impulse account and one empty project, **or** a decision not to make one
- [ ] `python scripts/selftest.py` ends `19 of 19 checks passed`
- [ ] **Bring your USB-C data cable**, your IMU on its four wires, and a charged laptop

## What you produce on Thursday

| File | From |
|---|---|
| `w10/sessionA.txt`, `w10/sessionB.txt` (or a neighbour's, named) | runs 1 and 2 |
| `w10_out/` — features, `feature_table.txt`, `manifest.json`, `cm_rule.json`, `cm_model.json` | runs 2b–3 |
| `w10/model_weights.h` — the one you flashed | run 3 |
| `w10/live.txt` | run 4 |
| `RECORD_SHEET.md`, filled in | all day |
| `AI_USAGE.md`, two entries | cycles 2 and 3 |

Plus the Google Form before you leave. HW5 (5% of the semester) is built on all of it; the
due date is in the announcement.

## Consent, in one paragraph

You are collecting motion data from a person — yourself, and possibly a neighbour. The file
carries no name, but a motion signature can identify a person, and "it is only IMU data" is
not consent. Section 0 of the record sheet is three lines: what is collected, who sees it,
when it is deleted. Fill it in before run 1. If you upload to Edge Impulse, the data goes to
a Qualcomm-owned service; that is a choice you make, not something the pack makes for you.
