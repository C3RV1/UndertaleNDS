import pathlib

import binary
import numpy as np
from PIL import Image
import json
import sys
import os


def convert(input_file, output_file):
    print(f"Converting {input_file} to {output_file}")
    with open(input_file, "r") as f:
        data = json.loads(f.read())

    image = Image.open(os.path.splitext(input_file)[0] + ".png")

    np_array = np.array(image)
    np_array_palette = np.zeros((np_array.shape[0], np_array.shape[1]), dtype=np.uint8)

    palette = [np.array([0, 255, 0])]  # r 0 g 255 b 0 - transparent

    for row in range(np_array.shape[0]):
        for col in range(np_array.shape[1]):
            color: np.ndarray = np_array[row][col][:3]
            color >>= 3

            if np_array[row][col][3] == 0:
                i = 0
            else:
                for i, color_ in enumerate(palette):
                    color_: np.ndarray
                    if all(color == color_):
                        break
                else:
                    palette.append(color)
                    i = len(palette) - 1

            np_array_palette[row][col] = i  # color 0 reserved for transparent

    palette = palette[1:]  # remove transparent color
    palette = np.array([c[0] + (c[1] << 5) + (c[2] << 10) for c in palette],
                       dtype=np.dtype(np.uint16).newbyteorder("<"))

    width, height = data["size"]
    frame_count = data["frameCount"]

    tile_w, tile_h = (width + 7) // 8, (height + 7) // 8

    def get_tile(frame_, tile_x_, tile_y_):
        tile_ = np.zeros((8, 8), np.uint8)
        for y in range(8):
            if tile_y_ * 8 + y >= height:
                break
            copy_length = min(8, width - tile_x_*8)
            tile_[y][:copy_length] = np_array_palette[tile_y_*8 + frame_ * height + y][tile_x_*8:tile_x_*8+copy_length]
        return tile_

    tiles = []
    for frame in range(frame_count):
        for tile_y in range(tile_h):
            for tile_x in range(tile_w):
                tiles.append(get_tile(frame, tile_x, tile_y))
    tiles = np.array(tiles)

    wtr = binary.BinaryWriter(open(output_file, "wb"))
    wtr.write(b"CSPR")
    file_size_pos = wtr.tell()
    wtr.write_uint32(0)
    wtr.write_uint32(4)  # Version
    wtr.write_uint16(width)
    wtr.write_uint16(height)

    wtr.write_uint8(len(palette))
    wtr.write(palette.tobytes())

    wtr.write_uint8(frame_count)
    wtr.write(tiles.tobytes())

    animations = data.get("animations", [])
    wtr.write_uint8(len(animations))
    for animation in animations:
        wtr.write_string(animation["name"], encoding="ascii")
        frames = animation["frames"]
        wtr.write_uint8(len(frames))
        for frame in frames:
            wtr.write_uint8(frame["frame"])
            wtr.write_uint16(frame["duration"])
            wtr.write_int8(frame.get("draw_off_x", 0))
            wtr.write_int8(frame.get("draw_off_y", 0))

    size = wtr.tell()
    wtr.seek(file_size_pos)
    wtr.write_uint32(size)
    wtr.close()


def compileSprites():
    for root, _, files in os.walk("spr"):
        for file in files:
            path = os.path.join(root, file)
            if path.endswith(".png"):
                continue
            path_dest = os.path.splitext(os.path.join("../nitrofs", path))[0] + ".cspr"
            if os.path.isfile(path_dest):
                src_time = os.path.getmtime(path)
                src_time2 = os.path.getmtime(os.path.splitext(path)[0] + ".png")
                dst_time = os.path.getmtime(path_dest)
                if src_time > dst_time:
                    convert(path, path_dest)
                elif src_time2 > dst_time:
                    convert(path, path_dest)
            else:
                pathlib.Path(os.path.split(path_dest)[0]).mkdir(exist_ok=True, parents=True)
                convert(path, path_dest)


if __name__ == '__main__':
    compileSprites()
