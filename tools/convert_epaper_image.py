#!/usr/bin/env python3
"""Convert a binary PPM image to Adafruit EPD black/red bitmap layers.

Use ffmpeg to make a 300x400 PPM first, then run this script. It writes both a
visual preview PPM and a C++ header containing two PROGMEM 1-bit bitmaps.
"""

import pathlib
import sys


def read_ppm(path):
    raw = pathlib.Path(path).read_bytes()
    if not raw.startswith(b"P6"):
        raise ValueError("Expected a binary PPM (P6) input")
    index = 2
    tokens = []
    while len(tokens) < 3:
        while raw[index:index + 1] in b" \t\r\n":
            index += 1
        if raw[index:index + 1] == b"#":
            index = raw.index(b"\n", index) + 1
            continue
        end = index
        while raw[end:end + 1] not in b" \t\r\n":
            end += 1
        tokens.append(int(raw[index:end]))
        index = end
    while raw[index:index + 1] in b" \t\r\n":
        index += 1
    width, height, max_value = tokens
    if max_value != 255:
        raise ValueError("Only 8-bit PPM files are supported")
    pixels = raw[index:]
    if len(pixels) != width * height * 3:
        raise ValueError("PPM pixel data has an unexpected length")
    return width, height, pixels


def classify(red, green, blue):
    # Keep true red as red. Treat dark blue/black and the source's yellow
    # ornamentation as black; all remaining light pixels become white.
    if red > 150 and green < 135 and blue < 135:
        return "red"
    saturated = max(red, green, blue) - min(red, green, blue) > 55
    luminance = (red * 30 + green * 59 + blue * 11) // 100
    return "black" if saturated or luminance < 185 else "white"


def write_header(path, name, width, height, black, red):
    row_bytes = (width + 7) // 8

    def pack(which):
        data = bytearray(row_bytes * height)
        for y in range(height):
            for x in range(width):
                if which[y * width + x]:
                    data[y * row_bytes + x // 8] |= 1 << (7 - (x % 8))
        return data

    def array_text(identifier, data):
        rows = []
        for offset in range(0, len(data), 16):
            rows.append("  " + ", ".join(f"0x{byte:02X}" for byte in data[offset:offset + 16]))
        return f"static const uint8_t {identifier}[] PROGMEM = {{\n" + ",\n".join(rows) + "\n};\n"

    content = "#pragma once\n#include <Arduino.h>\n\n"
    content += array_text(f"{name}_black", pack(black)) + "\n"
    content += array_text(f"{name}_red", pack(red))
    pathlib.Path(path).write_text(content)


def write_preview(path, width, height, colors):
    palette = {"black": (0, 0, 0), "red": (190, 0, 0), "white": (255, 255, 255)}
    data = bytearray()
    for color in colors:
        data.extend(palette[color])
    pathlib.Path(path).write_bytes(f"P6\n{width} {height}\n255\n".encode() + data)


def main():
    if len(sys.argv) != 5:
        raise SystemExit("Usage: convert_epaper_image.py INPUT.ppm OUTPUT.h PREVIEW.ppm NAME")
    source, header, preview, name = sys.argv[1:]
    width, height, pixels = read_ppm(source)
    if (width, height) != (300, 400):
        raise SystemExit(f"Expected 300x400 input, got {width}x{height}")
    colors = [classify(*pixels[offset:offset + 3]) for offset in range(0, len(pixels), 3)]
    write_header(header, name, width, height,
                 [color == "black" for color in colors],
                 [color == "red" for color in colors])
    write_preview(preview, width, height, colors)


if __name__ == "__main__":
    main()
