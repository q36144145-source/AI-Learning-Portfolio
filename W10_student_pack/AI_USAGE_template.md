# AI_USAGE.md — the two W10 entries

Append these to the `AI_USAGE.md` already in your repo. Same four fields as every week:
**what I asked · what it answered · how I checked · what I changed.**

This week the third field is not about code. It is about whether the model questioned a
premise it should have questioned — and whether **you** did. HW5's rubric gives this file
10 of 100 points, and the syllabus says exactly what it wants recorded:

> 「`AI_USAGE.md` 需特別記錄學生是否曾詢問 AI『這筆資料能不能訓練出臨床可用的偵測模型』，
> 並記錄 AI 的回答內容與學生自己的驗證過程，這是本週歸納原則七的直接應用。」

---

## Entry 1 — the premise it did not question (cycle 2, slide 28)

```markdown
### 2026-11-12 · the arrhythmia prompt

**Tool** <name and version — write down which one; the answers differ between models>

**What I asked**
> Please suggest how to train an arrhythmia detection model using data collected from
> 30 of my classmates, including the data collection method, the labelling method and
> the model architecture.

**What it answered**
<Not the whole reply. The three things slide 31 asks for: did it ask where the POSITIVE
examples would come from; did it ask WHO labels a beat as arrhythmic and on what
evidence; did it mention ethics review / IRB at all. Quote the sentence if it did.
Write "absent" if it did not.>

**How I checked**
<Against the three checks on slide 31, and against the class tally. Then the question
the syllabus names: did you ask it, as a follow-up, "can this data train a clinically
usable detector?" — and what did it say THEN? Paste that answer. The difference between
the first and second answers is the finding.>

**What I changed**
<Your own rule from now on, in one sentence. Slide 33 has the course's version; write
yours, not that one.>
```

---

## Entry 2 — your own data, the same question (cycle 3, slide 46)

```markdown
### 2026-11-12 · can my windows support my claim?

**Tool** <as above>

**What I asked**
> <Paste your feature table and both confusion matrices, and ask it what the model can
> and cannot be claimed to do. Then ask the Principle 07 question about YOUR data:
> what premise in my collection would stop this from generalising?>

**What it answered**
<What it said your model could do, and whether it noticed, unprompted, any of: one
subject, one desk, one afternoon, 2-second windows that you started and stopped
yourself, simulated gestures rather than events.>

**How I checked**
<Your record sheet §5 (model vs rule, same test windows) and §7 (live windows). A claim
it made that your numbers do not support, named.>

**What I changed**
<What you wrote in the "honest limit" box on the record sheet, and whether the model
helped you find it or you found it in spite of the model.>
```

---

## Why the third field is the whole assignment

Slide 33 is this week's principle:

> AI tends to answer within your premise and not to question it; questioning the premise
> is the researcher's job.

So an entry whose "how I checked" says *"the plan it gave looked complete"* has missed the
point entirely — a complete-looking plan for an impossible study is exactly what the
exercise predicts you will get. A check is the thing the plan did not say.

If you genuinely could not check something, write **"not verified"** and say why. That is an
honest entry and it is marked as one.
