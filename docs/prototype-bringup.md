# Prototype bring-up and validation

Target: Daisy Seed/Seed3 (STM32H750) module with external SDRAM. This is **not** a bare STM32H750 board design. Firmware compiles and host DSP tests exist; **hardware flash, codec audio, and DMA timing have not yet been verified**.

## Before PCB fabrication

- [ ] Confirm the exact Daisy module revision, power rails, codec pins and audio voltage limits against the official module documentation.
- [ ] Design a high-impedance guitar input buffer (nominally 1 MΩ), safe codec input level and appropriate output driver. Do not connect guitar jack tip to an MCU GPIO.
- [ ] Ensure accessible BOOT, RESET, USB, GND, power and codec audio test points.
- [ ] Connect 10 kΩ linear pots: Time A0, Mix A1, Feedback A2, Tone A3; each across 3.3 V and GND.
- [ ] Connect momentary Bypass D0 and Tap D1 to GND; latching Mode D2 to GND. Firmware enables internal pull-ups.
- [ ] Check analog and digital ground return, power supply noise and physical jack wiring.
- [ ] Prefer a reworkable breakout or prototype board for first audio verification.

## First power-on

- [ ] Inspect PCB for shorts, reversed connectors and incorrect supply voltages before connecting audio equipment.
- [ ] Power from a documented, current-limited source and verify supply rails.
- [ ] Record the Git commit SHA and download the matching `daisy_delay.bin` from a successful CI run.
- [ ] Flash via USB DFU; verify boot and onboard LED.
- [ ] Start at low monitored output volume with a suitably attenuated test signal into the conditioned codec input.
- [ ] Confirm that the left codec input appears at both outputs. Startup mode is DSP bypass; press Bypass once to enable delay.

## Audio validation matrix

Record measured results with hardware revision, firmware SHA, supply, test signal and audio capture.

| Check | Procedure | Expected | Status |
| --- | --- | --- | --- |
| Dry signal | Bypass, 1 kHz test tone | Stable signal, document gain and noise | Not tested |
| Echo timing | Enable, wet mix, minimum feedback, 350 ms | One repeat near 350 ms | Not tested |
| Delay range | Sweep Time | Approximately 20–2000 ms | Not tested |
| Feedback | Gradually increase | Decaying repeats, no runaway clipping | Not tested |
| Tone | Compare minimum and maximum | Repeats lose more high frequencies at minimum | Not tested |
| Tap | Tap every 500 ms | Approximately 500 ms quarter-note repeats | Not tested |
| Mode | Change time in both modes | Crossfade versus pitch-glide behavior | Not tested |
| Bypass | Toggle while echoing | Dry signal remains, measure switching clicks | Not tested |
| Endurance | Continuous audio, 30+ minutes | No hangs, dropouts or abnormal heating | Not tested |

The existing bypass is **DSP bypass**, not power-off-safe relay true bypass.

## Reporting and iteration

For every defect, create an issue with steps to reproduce, expected/actual output, firmware SHA, hardware revision and measurements or audio capture. Fix through a dedicated branch and PR, with automated regression tests where practical. Never label physical hardware behavior as validated solely because CI passes.

## First prototype record

- Module revision: pending
- PCB revision: pending
- Firmware SHA: pending
- Input/output analog circuit: pending
- Hardware flash: not tested
- Codec audio and DMA: not tested
- Audio measurements: unavailable
