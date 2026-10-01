# Before class on Thursday 1 Oct

Four items. Two of them are free. The other two cost you the morning if they are missing.

- [ ] **1 · Collect your two resistors and two capacitors** from the TA table
- [ ] **2 · Download the AD8232 datasheet** and put the PDF somewhere you can open offline
- [ ] **3 · Find your W03 numbers** and have them open
- [ ] **4 · Check the toolchain still works** — same as W03, nothing new

---

## 1 · Two resistors and two capacitors

Pick up from the TA table, in one small bag:

| | Value | How to recognise it |
|---|---|---|
| Resistor | 10 kΩ | four colour bands: **brown black orange** gold |
| Resistor | 100 kΩ | four colour bands: **brown black yellow** gold |
| Capacitor | 100 nF | a small disc printed **`104`** |
| Capacitor | 10 nF | a small disc printed **`103`** |

**The capacitor printed `104` is the 100 nF one, and `103` is the 10 nF one.** The three
digits are picofarads written as two digits and a power of ten, so `104` means
10 × 10⁴ pF = 100 000 pF = 100 nF. Getting the two discs the wrong way round swaps your two
configurations without changing anything you can see, and the two runs then look like a
result. Sort them into two labelled pairs tonight: **10 kΩ with `104`**, and
**100 kΩ with `103`**.

If your resistors happen to be printed with digits instead of banded, the same code is used
but the base unit is ohms: a resistor marked `103` is 10 kΩ and one marked `104` is 100 kΩ —
the exact opposite pairing to the capacitors. Read the body, not the number.

You already have the breadboard and the jumper wires from W02.

## 2 · The AD8232 datasheet

Cycle 3 is a paper exercise on a chip we could not buy, and it needs the original document,
not a summary of it.

Go to Analog Devices' own site, search for **AD8232**, and download the **data sheet** PDF
(not the evaluation-board user guide, not a distributor's product page). Save it where you
can open it with no internet — the room's wifi with forty-five people on it is not a plan.

The direct link, as of the time this pack was written, is
`https://www.analog.com/media/en/technical-documentation/data-sheets/ad8232.pdf`.
**If that 404s, search the site rather than taking a copy from anywhere else.** The whole
exercise is about whether a claim can be traced to the manufacturer's own document, and a
PDF from a random file host cannot do that job.

## 3 · Your W03 numbers

Open your `w03/RECORD_SHEET.md` and confirm you can still find:

- the **standard deviation** of your sampling interval for the timer version, in ms
- the same for the `delay(4)` version

HW2 requires you to carry those forward. If you cannot find them, say so in the record sheet
rather than leaving the row blank — and tell a TA at the start of class, because there is a
way to recover it and it is easier at 09:15 than at 11:45.

## 4 · The toolchain

Nothing new this week. Confirm the W03 project still builds:

```bash
python -m platformio run
```

and that Python still has what it needs:

```bash
python -c "import numpy, matplotlib; print('ok')"
```

If `numpy` is still not installed, **come anyway**. `adcstats.py` is standard library only,
so all of cycle 1 works without it. It is cycle 2 that needs numpy, and a TA can fix it in
five minutes at the start of the morning.

## Bring, as always

A USB-C cable you know carries data, and the board. Plug into the socket the TAs showed you
in W03 — this board has two and only one of them gives you a serial port.

## If you get stuck

Post in the course channel with your OS, the command you ran, and the **full** error
message. Then stop and come to class.
