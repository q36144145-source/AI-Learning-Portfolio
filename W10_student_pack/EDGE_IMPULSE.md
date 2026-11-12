# Edge Impulse — the L1 path, and what to do when it is down

**Embedded Systems (BME 3301) · Week 10 · Thursday 12 Nov**

Edge Impulse Studio is the graphical tool this course uses once, this week, at "use" level:
you are not asked to read what it generates. It is a **web service**. Forty-five people on one
classroom network, each uploading sixty-four files and starting a training job, is a real risk
to the session, which is why:

1. **Nothing in cycle 1 touches the network.** Your data is collected on your board, through
   the serial port, into a log on your laptop. `windows.py` turns it into Edge Impulse files
   on your disk. If the network is down you still have everything.
2. **The offline path is the one the pack guarantees.** `scripts/train.py` builds the same
   kind of model from the same windows, scores it on the same test windows, and writes the
   header `sketch_c_classify.cpp` runs. Cycle 3 does not depend on Edge Impulse at all.
3. **Your account is made before Thursday**, not at 09:40. See `PREFLIGHT.md`.

> **Honesty note.** The steps below were written from Edge Impulse's documentation as it stood
> in early October 2026, not from a run in this classroom. The TA dry run on Wed 11 Nov walks
> them on a real account; if a page or button has moved, the corrected version is posted in the
> course channel before class. Menu names change; the shape of the workflow does not.

---

## What the free plan is, and who owns it

- The **Developer Plan** is free and is the default tier. Edge Impulse's announcement of it
  (6 May 2025) lists "GPU access and 60-minute training jobs", "three private projects" with
  "up to 10 experiments each", and "up to 3 collaborators per project".
  <https://www.edgeimpulse.com/blog/introducing-the-developer-plan/>
- The same page lists the company as "Edge Impulse, Inc. c/o Qualcomm Incorporated". Edge
  Impulse was acquired by **Qualcomm in 2025**. Slide 22 is about why that matters to you.
- One project is all this week needs. A sixty-four-window dataset trains in well under a
  minute of the sixty you are allowed.

## Before Thursday (PREFLIGHT item 4)

1. Create a free account at <https://studio.edgeimpulse.com>. Use an address you are happy to
   have on a Qualcomm-owned service — that is a data decision, and it is yours.
2. Create one project. Call it `bme3301-w10`. Leave it empty.
3. Log out and log back in once, so you know the password works on Thursday.

## In the room: what is guaranteed, what is attempted

- **Guaranteed:** the Studio workflow is shown from the front, on a project the TAs built on
  11 Nov from their own dry-run sessions. You will see every page — upload, impulse, training,
  Model testing — even if the room's network gives nothing to anyone.
- **Attempted:** your own upload, inside the ten-minute block and never on the critical path.
  If it is slow, stop and run `train.py`; the header cycle 3 flashes comes from there anyway.
- **Finished at home, if you want the Studio column on sheet §4:** the same two folders,
  uploaded from a network that is not shared with forty-four other people.

## In class — cycle 2, after `windows.py` has written `w10_out/ei_export/`

### Upload (slide 25)

1. Open your project → **Data acquisition** → **Upload data**.
2. Choose the files in `w10_out/ei_export/training/` (select all of them, or select the
   folder) and set the category to **Training**. Upload.
3. Repeat with `w10_out/ei_export/testing/`, category **Testing**.
4. **Check the labels.** Edge Impulse infers the label from the filename, "through the
   following regex: `^[a-zA-Z0-9\s-_]+`" — that is, everything before the first dot. The files
   are named `shake.sessionA.w07.json`, so the label is `shake`. Four labels should appear,
   with the counts `windows.py` printed. If a label looks like `shake.sessionA`, the file was
   renamed; do not continue.
5. Each file is **one** two-second window, 100 Hz, six axes in `m/s2` and `dps`. If the
   upload rejects the gyroscope unit string, re-run `windows.py` with `--ei-accel-only` and
   upload those files instead; say so on your record sheet, because the model then sees three
   axes where the offline model sees six.

**What you did NOT do:** let the Studio split your data. "Automatically split" is the trap on
slide 20 — your test session is a different sitting, and the category you chose on upload
keeps it that way.

### Design and train (slide 26)

1. **Create impulse.** Input block: *Time series data*, window size **2000 ms**, window
   increase **2000 ms**, frequency **100 Hz**. Each file is exactly one window; a smaller
   stride would manufacture overlapping windows out of the same two seconds.
2. Add a processing block: **Spectral features** (the one the deck calls "Spectral Analysis").
   Add a learning block: **Classification**. Save.
3. Open **Spectral features** → **Generate features**. The feature explorer shows your windows
   in colour. If one class is a tight blob far from the rest, that is the class your rule
   probably already gets right.
4. Open **Classifier** → **Start training**, with the default settings. Today is about the
   workflow, not the hyper-parameters. Note the number of epochs and the learning rate it used.
5. While it trains: the AI unit (slides 28–31). Do not sit watching the bar.

### Test (slide 34)

1. **Model testing** → **Classify all**. This runs the model on the **Testing** category —
   your other session, the windows `baseline.py` scored the rule on.
2. Copy the 4×4 matrix into record sheet §4, rows = true class in `still, tilt, shake, tap`
   order. Then feed it to `compare.py` with `--manual`, so it sits next to the rule's matrix:

   ```bash
   python scripts/compare.py w10_out/cm_rule.json w10_out/cm_model.json \
          --manual "a,b,c,d;e,f,g,h;i,j,k,l;m,n,o,p" --manual-name edge-impulse
   ```

   `compare.py` checks that the manual matrix counts the same number of windows as the test
   set. If it does not, you did not upload exactly the `testing/` folder.

### Export — for W11, not for today (slide 49)

1. **Deployment** → **Arduino library**. Before you download, write down the **estimated
   RAM and Flash** the page shows for the unquantised (float32) model — W11 compares them
   against the int8 build.
2. Download the `.zip` and keep it in your repo under `w10/`. You are **not** asked to flash
   it today: cycle 3 runs the offline model, which is the one this pack can build. If you want
   to run the Edge Impulse library on the board at home, the zip contains its own example
   sketch; it is not part of HW5's required deliverables unless the announcement says so.

## If it is down, slow, or you cannot log in

Nothing is lost. Run the offline path — it is what cycle 3 uses anyway:

```bash
python scripts/train.py w10_out --header sketches/model_weights.h
python scripts/compare.py w10_out/cm_rule.json w10_out/cm_model.json
```

Record sheet §4 has a row for each. Write which one you used and why.
