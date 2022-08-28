import numpy as np
from PIL import Image
import sys
import os
import binary
from xml.etree import ElementTree as ET


def main():
    if len(sys.argv) != 2:
        return
    fp = sys.argv[1]
    out = os.path.splitext(fp)[0] + ".cfnt"

    glyph_map = np.array([0] * 256, dtype=np.uint8)

    et = ET.parse(fp)
    root = et.getroot()
    image_fp = None
    glyphs = None
    for child in root:
        if child.tag == "glyphs":
            glyphs = child
        if child.tag == "image":
            image_fp = os.path.join(os.path.dirname(fp), child.text)

    img = Image.open(image_fp)

    wtr = binary.BinaryWriter(open(out, "wb"))

    wtr.write(b"CFNT")
    file_size_pos = wtr.tell()
    wtr.write_uint32(0)
    wtr.write_uint32(1)

    line_height = 0
    glyph_count = 0
    for glyph in glyphs:
        character_id = int(glyph.attrib["character"])
        if character_id >= 256:
            continue
        glyph_map[character_id] = glyph_count + 1
        glyph_count += 1
        line_height = max(line_height + 1, int(glyph.attrib["h"]))

    wtr.write_uint8(line_height)
    wtr.write_uint8(glyph_count)
    for glyph in glyphs:
        character_id = int(glyph.attrib["character"])
        if character_id >= 256:
            continue
        x = int(glyph.attrib["x"])
        y = int(glyph.attrib["y"])
        w = int(glyph.attrib["w"])
        h = int(glyph.attrib["h"])
        wtr.write_uint8(w // 2)
        wtr.write_uint8(h // 2)
        wtr.write_uint8(int(glyph.attrib["shift"]) // 2)
        wtr.write_uint8(int(glyph.attrib["offset"]))
        byte = 0
        bit_pos = 0
        for glyph_y in range(h // 2):
            for glyph_x in range(w // 2):
                pixel = img.getpixel((x + glyph_x * 2, y + glyph_y * 2))
                if pixel[3] > 0:
                    byte += 1 << (7 - bit_pos)
                bit_pos += 1
                if bit_pos == 8:
                    wtr.write_uint8(byte)
                    byte = 0
                    bit_pos = 0
        if bit_pos != 0:
            wtr.write_uint8(byte)

    wtr.write(glyph_map.tobytes())

    size = wtr.tell()
    wtr.seek(file_size_pos)
    wtr.write_uint32(size)
    wtr.seek(size)

    wtr.close()


if __name__ == '__main__':
    main()
