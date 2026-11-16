#!/usr/bin/env python3
"""Check that the K3 Max VIA definitions match the firmware they describe.

VIA picks a definition by USB VID/PID and maps every key it draws to a matrix position, so for each variant
keyboards/keychron/k3_max/via_json/k3_max_<layout>_<backlight>.json must have:
- the VID/PID of the firmware (info.json + <layout>/<backlight>/keyboard.json),
- the size of the firmware matrix,
- exactly the matrix positions of the variant's LAYOUT_<layout>_* in info.json,
- lighting menus for the backlight it was built with (RGB matrix on rgb, none of it on white).
"""
import json
import sys
from pathlib import Path

LAYOUTS = ('ansi', 'iso', 'jis')
BACKLIGHTS = ('rgb', 'white')
RGB_MATRIX_CHANNEL = 3


def merge(base, extra):
    """Deep merge like QMK does for info.json / keyboard.json, extra wins."""
    out = dict(base)
    for key, value in extra.items():
        if isinstance(value, dict) and isinstance(out.get(key), dict):
            out[key] = merge(out[key], value)
        else:
            out[key] = value
    return out


def firmware_info(root, layout, backlight):
    info = {}
    folder = root
    for part in ('', layout, backlight):
        folder = folder / part if part else folder
        for name in ('info.json', 'keyboard.json'):
            path = folder / name
            if path.is_file():
                info = merge(info, json.loads(path.read_text(encoding='utf-8')))
    return info


def via_positions(via):
    """Matrix positions drawn by VIA: the first legend line of every key is "row,col"."""
    positions = []
    for row in via['layouts']['keymap']:
        for key in row:
            if isinstance(key, str):
                first = key.split('\n')[0]
                r, c = first.split(',')
                positions.append((int(r), int(c)))
    return positions


def channels(node):
    """Every channel id used by a VIA menu control, as ["id_name", channel, value]."""
    found = set()
    if isinstance(node, dict):
        content = node.get('content')
        if isinstance(content, list) and len(content) == 3 and isinstance(content[0], str) and isinstance(content[1], int):
            found.add(content[1])
        for value in node.values():
            found |= channels(value)
    elif isinstance(node, list):
        for value in node:
            found |= channels(value)
    return found


def check(root, layout, backlight):
    errors = []
    path = root / 'via_json' / f'k3_max_{layout}_{backlight}.json'
    try:
        via = json.loads(path.read_text(encoding='utf-8'))
    except (OSError, ValueError) as e:
        return [f'{path}: {e}']

    info = firmware_info(root, layout, backlight)

    if int(via['vendorId'], 16) != int(info['usb']['vid'], 16):
        errors.append(f"vendorId {via['vendorId']} != firmware VID {info['usb']['vid']}")
    if int(via['productId'], 16) != int(info['usb']['pid'], 16):
        errors.append(f"productId {via['productId']} != firmware PID {info['usb']['pid']}")

    rows, cols = len(info['matrix_pins']['rows']), len(info['matrix_pins']['cols'])
    if (via['matrix']['rows'], via['matrix']['cols']) != (rows, cols):
        errors.append(f"matrix {via['matrix']['rows']}x{via['matrix']['cols']} != firmware {rows}x{cols}")

    names = [name for name in info['layouts'] if name.startswith(f'LAYOUT_{layout}_')]
    if len(names) != 1:
        errors.append(f'expected one LAYOUT_{layout}_* in info.json, found {names}')
    else:
        firmware = sorted(tuple(key['matrix']) for key in info['layouts'][names[0]]['layout'])
        drawn = via_positions(via)
        duplicates = sorted({p for p in drawn if drawn.count(p) > 1})
        if duplicates:
            errors.append(f'keys drawn twice: {duplicates}')
        if sorted(set(drawn)) != firmware:
            missing = sorted(set(firmware) - set(drawn))
            extra = sorted(set(drawn) - set(firmware))
            errors.append(f'keys do not match {names[0]}: missing {missing}, not in the firmware {extra}')

    used = channels(via.get('menus', []))
    if backlight == 'rgb' and RGB_MATRIX_CHANNEL not in used:
        errors.append('RGB variant without RGB matrix menus')
    if backlight == 'white' and RGB_MATRIX_CHANNEL in used:
        errors.append('white variant with RGB matrix menus')

    return [f'{path.name}: {e}' for e in errors]


def main():
    root = Path(sys.argv[1] if len(sys.argv) > 1 else 'keyboards/keychron/k3_max')
    errors = []
    for layout in LAYOUTS:
        for backlight in BACKLIGHTS:
            found = check(root, layout, backlight)
            print(f"k3_max_{layout}_{backlight}.json: {'ok' if not found else 'FAILED'}")
            errors += found
    for e in errors:
        print(f'::error::{e}')
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main())
