# Keychron K3 Max firmware

Unofficial QMK firmware for the Keychron K3 Max. It starts from Keychron's own firmware (Keychron
`qmk_firmware`, branch `2025q3`) and brings it up to date with upstream QMK. It is not made or supported by
Keychron.

## What is different from the stock firmware

- **Current QMK**: Keychron 2025q3 is built on QMK as of 2025-09-10; this firmware merges QMK as of 2026-11-15.
  Every submodule comes from QMK, ChibiOS included (HAL 9.1 instead of Keychron's 8.4): the Keychron ChibiOS
  fixes the K3 Max needs live in [`keyboards/keychron/k3_max`](keyboards/keychron/k3_max/).
- **Raw HID over Bluetooth and 2.4 GHz**, not only over USB, with and without VIA.
- **Battery readout**: raw HID command `0xA7 0xF0` returns level, voltage, charging state, transport and wireless
  state.
- **The key that wakes the keyboard is not lost** over Bluetooth and 2.4 GHz: after a pause of at least 10
  seconds, the keys pressed while the link is down are sent once it is back, if that takes less than 5 seconds.
  When the link drops while typing or switching host, the keys are still dropped instead of arriving late.
- **Release number as firmware version** in the Keychron web tool.
- Fixes to the EEPROM emulation and to the build of the ANSI RGB and white-backlight variants.

Keymaps, layouts, lighting effects and USB identity (VID/PID, device version 1.1.0) are the stock ones. The
[v0.0.1 release notes](.github/release-notes/v0.0.1.md) list every change in detail, including the upstream QMK
fixes that matter for this keyboard and the battery reply format.

## Download

Every release on the [Releases](https://github.com/Freedye/keychron-k3-max-qmk/releases) page has:

- six firmware files, `k3_max_keychron_<version>_<backlight>_<layout>.bin`, with their SHA-256 in the notes.
  Pick your keyboard: layout `ansi`, `iso` or `jis`, backlight `rgb` or `white`;
- the six VIA definitions, `k3_max_<layout>_<backlight>.json`.

## Flash

1. Set the mode switch to **Cable**.
2. Unplug the keyboard, then hold **Esc** (or the reset button under the space bar) while plugging the USB cable
   back in. The keyboard starts in DFU mode, shown as *STM32 BOOTLOADER* (USB ID `0483:df11`).
3. Flash the `.bin` with [QMK Toolbox](https://github.com/qmk/qmk_toolbox), or with `dfu-util`:

   ```bash
   dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D k3_max_keychron_v0.0.1_rgb_iso.bin
   ```

   On Windows the DFU device needs the WinUSB driver: QMK Toolbox installs it, or use Zadig.

The DFU bootloader is in the MCU ROM and cannot be overwritten, so a failed flash or a wrong variant is fixed by
entering DFU mode again and flashing the right file. Updating from the Keychron Launcher installs the stock
firmware over this one.

## Check what is flashed

The Keychron Launcher shows the firmware version as `<version> <build date>`:

| Version | Firmware |
|---|---|
| `v0.0.1`, `v0.0.2`, ... | A release from this repository |
| `dev-<commit>` | A CI build of a branch that is not `master` |
| `dev` | A local build |

## Configure

- **[Keychron Launcher](https://launcher.keychron.com/)**: everything, including the Keychron-only settings
  (per-key RGB, mixed lighting, debounce, wireless settings).
- **[VIA](https://usevia.app/)**: keymap, layers, macros and lighting. If VIA does not recognize the keyboard,
  load the definition of your variant from the release: Settings → Show Design tab, then Design → Load.
- **Your own tools**: raw HID, over the cable or wireless, for example to read the battery.

Both web tools need a Chromium-based browser; use them with the keyboard on the USB cable.

## Build

```bash
git clone --recurse-submodules https://github.com/Freedye/keychron-k3-max-qmk.git
cd keychron-k3-max-qmk
python3 -m pip install qmk
qmk setup -H .
qmk compile -kb keychron/k3_max/iso/rgb -km keychron
```

Replace `iso/rgb` with your variant. Local builds report `dev` as firmware version;
`-e KC_FIRMWARE_VERSION=<name>` sets another one (at most 10 characters).

## CI and releases

Every push runs the [K3 Max CI](.github/workflows/build_k3_max.yml):

- commit hygiene: whitespace and conflict markers in the pushed commits, submodules from QMK only;
- `qmk lint` of the six variants, and the VIA definitions checked against the firmware (VID/PID, matrix, keys,
  lighting menus);
- the QMK unit tests for what the K3 Max uses: key and layer handling, mouse keys, housekeeping, wear leveling;
- a build of the six variants, each image checked for size, vector table, the K3 Max ChibiOS fixes and the
  firmware version, plus a build of the default keymap.

Every green push to `master` becomes a release: `v0.0.1` first, then the patch number goes up by one.

## License

[GNU General Public License v2.0](LICENSE). Based on [Keychron's QMK firmware](https://github.com/Keychron/qmk_firmware)
and on [QMK](https://github.com/qmk/qmk_firmware).
