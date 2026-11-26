# W11 In-class exercise — where do the bytes live?

**Name** 陳竑勳  **Student ID** Q36144145

**Hand this in before the reveal slide. 8 minutes. 10 points.** Not part of the
semester grade; the class distribution is read back to you next week as a diagnostic.

---

A language model of **28.9 million parameters** is to run on **your board**: 512 KB
internal SRAM, 8 MB PSRAM, 16 MB flash. Its author reports that 25 million of those
parameters are a lookup table, and that about 6 rows of that table, roughly 450 bytes,
are read per generated token. (slvDev, *esp32-ai*, README, read 5 Oct 2026.)

You have, from this morning, on this board:

- run 1 — ns per word for a sequential and a random read, in all three tiers;
- run 2 — µs per inference for one dense layer whose weights were read once, in all
  three tiers;
- run 3 — µs per token for six random 64-byte rows out of a 1 MB table, flash and PSRAM.

Three components. For each, **choose one tier and give the reason**. The reason must
use **both** factors — *how often it is read per token* and *how many bytes* — and must
cite **one of your own measured figures or ratios**. "It is big, so flash" scores half.

| Component | Parameters / size | Read per token | Your tier | Your reason (1–2 sentences, with a number of yours) |
|---|---|---|---|---|
| **Embedding lookup table** | 25 M parameters; ~6 rows (~450 B) touched per token | a few rows, by address | **Flash** | Only flash (16 MB) can hold ~12.5 MB, and only ~6 rows (a few hundred bytes) are read per token, by address; my 6-row lookup from a 1 MB flash table took 14.91 µs/token, 83× less than one int8 dense layer from SRAM (1242.3 µs), so the slowest tier is not the bottleneck. |
| **Dense core and output head** | the remaining ~3.9 M parameters (28.9 M − 25 M); every weight read once per token | all of it, sequentially, once | **PSRAM** | All ~3.9 MB is read once, sequentially, every token, and it does not fit in SRAM (largest internal block 335,860 B); my sequential read cost 2.47× SRAM in PSRAM vs 5.14× in flash, and my f32 dense layer was 2.22× slower from flash than from PSRAM. (With my scalar int8 kernel the tier made no difference, 1.00×, because compute dominated; the choice matters once a SIMD kernel makes memory the bottleneck.) |
| **Activations and normalisation weights** | kilobytes | many times per token, read **and written** | **SRAM** | Only kilobytes, but read and written many times per token, and flash cannot be written at run time; SRAM had the lowest random-read cost I measured, 147 ns/word against 3.31× in PSRAM and 6.48× in flash. |

**One more line.** The 25 M-parameter table at 4 bits per parameter is about 12.5 MB.
Which of the three tiers can physically hold it? Cross out the ones that cannot:

~~SRAM (512 KB)~~ · ~~PSRAM (8 MB)~~ · **flash (16 MB)** — only flash can hold 12.5 MB.

---

### How it is scored

| | Points | Full marks when |
|---|---|---|
| The criterion | 4 | every reason uses *frequency × bytes*, not one factor alone, and cites a figure you measured |
| Lookup table | 2 | a tier that can hold it, and the reason names the sparsity — few bytes per token, by address — not just the size |
| Dense core + head | 2 | the reason says *read once, all of it, sequentially* and ties that to your run-1 or run-2 sequential ratios |
| Activations + norms | 2 | the reason says *many times per token, and written*, and ties that to the tier whose random access cost you measured lowest |

You are not marked on whether your map matches the author's. You are marked on whether
the reasons would survive a different model with a different parameter split.
