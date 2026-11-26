# Where the two reading files came from

Both files are **verbatim copies** of files in Espressif's ESP-NN repository, fetched on
**5 Oct 2026** from the `master` branch at commit `d9be5422a79f4115e4275452dcaba3fe363a697d`
(committed 2026-09-22). License: Apache-2.0, as stated in each file's header.

| File here | Source path in `github.com/espressif/esp-nn` |
|---|---|
| `esp_nn_fully_connected_ansi.c` | `src/fully_connected/esp_nn_fully_connected_ansi.c` |
| `esp_nn_fully_connected_s8_esp32s3.S` | `src/fully_connected/esp_nn_fully_connected_s8_esp32s3.S` |

Nothing in either file was edited. The slide in cycle 3 shows a continuous slice of each,
with the source line numbers in the slide's subtitle, so you can open the file here and
read the lines before and after.

**What you are asked to do with them:** read. Point at the lines that do the same job
in both — the multiply-accumulate — and say what the SIMD version does per instruction
that the C version does per iteration. You are **not** asked to write, modify, or run
any assembly. That is the course's L4 boundary and it is deliberate.

**What ESP-NN says about itself** (its README, same commit): on the ESP32-S3 at 240 MHz
with data in internal memory, the optimised `fully connected` kernel took 1,543 cycles
against 8,439 for the ANSI C version on its test case (`len: 271, ch = 3`) — a ratio
it reports as 5.47. That is Espressif's measurement of Espressif's code on Espressif's
test; it is quoted on the slide as such, and it is not a number you can transfer to your
layer without measuring.

**Note on the file name.** The `.S` file is named `_s8_` but the function it defines is
`esp_nn_fc_s16_esp32s3` (the inputs are widened to 16-bit before the vector
multiply-accumulate). That is how the file is in the repository; it is not a copying
error.
