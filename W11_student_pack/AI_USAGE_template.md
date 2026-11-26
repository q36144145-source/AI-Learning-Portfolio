# AI_USAGE.md — the two W11 entries

Append these to the `AI_USAGE.md` already in your repo. Same four fields as every
week: **what I asked · what it answered · how I checked · what I changed.**

The third field is the one that is marked. This week both entries are about the same
failure in two sizes: an answer that sounds exactly right and was never checked against
anything that could contradict it.

---

## Entry 1 — the typical number (cycle 2)

```markdown
### 2026-11-19 · int8 versus float32 on an ESP32-S3

**Tool** <name and version>

**What I asked**
> On an ESP32-S3, how much faster is int8 inference than float32 for a fully
> connected layer? Give a number.

**What it answered**
<The number it gave, and every qualifier it attached to it. If it refused to give a
number, say so - that is also an answer.>

**How I checked**
<Your run 2: f32 sram / s8 sram = ____, f32 flash / s8 flash = ____. Say whether
the model's number was near either, and - more important - whether it said which
memory the weights were in. A speed-up without a tier is not a number you can use.>

**What I changed**
<What you would now write in a report instead of the model's figure.>
```

## Entry 2 — the premise it did not question (cycle 3, slides 41–44, carried over from W10)

W10 skipped this unit in class; it runs this week. If you already did it at home for W10, do it
again and add the class tally — W11 is online, so the TAs post the tally on Moodle after
25 Nov; fill it in then.

```markdown
### 2026-11-19 · the arrhythmia prompt

**Tool** <name and version — write down which one; the answers differ between models>

**What I asked**
> Please suggest how to train an arrhythmia detection model using data collected from
> 30 of my classmates, including the data collection method, the labelling method and
> the model architecture.

**What it answered**
<Not the whole reply. The three things slide 44 asks for: did it ask where the POSITIVE
examples would come from; did it ask WHO labels a beat as arrhythmic and on what
evidence; did it mention ethics review / IRB at all. Quote the sentence if it did.
Write "absent" if it did not.>

**How I checked**
<Against the three checks on slide 44, and against the class tally the TAs post on Moodle (out of ___ answers:
positives absent ___, gold standard absent ___, IRB absent ___). Then the follow-up:
"Can this data train a clinically usable detector?" — paste what it said THEN. The
difference between the first and second answers is the finding.>

**What I changed**
<Your own rule from now on, in one sentence. W10's Principle 07 has the course's
version; write yours, not that one.>
```

### The 30-second second test — do it, log the result in entry 2

Same tool, three fact questions, no preparation: *What is the relationship between
TFLite Micro and LiteRT for Microcontrollers? Which company owns Edge Impulse now? Is
Apache microTVM still maintained?* Write down what it said and the date of the most
recent event it seems to know about. These are not hard questions. They are **recent**
questions. That is Principle 08 (slide 45).

---

## Why the third field is the whole assignment

Both entries this week sound authoritative: a speed-up figure for a chip the model has
never run on, and a complete study plan built on a premise nobody questioned. **Nothing in
the text tells you which parts it actually knows.** A check is something that can
contradict the answer — your run 2 ratio, the class tally, a sentence in a primary
source. "It sounded right" is not a check; it is the exact failure this unit is built to
demonstrate.

If you genuinely could not check something, write **"not verified"** and say why. That
is an honest entry and it is marked as one.
