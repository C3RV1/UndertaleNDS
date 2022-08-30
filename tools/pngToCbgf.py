#!/usr/bin/python3
import os
import pathlib

from PIL import Image
import sys
import numpy as np
import binary


FORCE_8BIT = True


def convert(input_file, output_file):
    print(f"Converting {input_file} to {output_file}")
    image = Image.open(input_file)
    np_array = np.array(image)  # access np_array[y][x]
    np_array_palette = np.zeros((np_array.shape[0], np_array.shape[1]), dtype=np.uint8)

    palette = [np.array([0, 255, 0])]  # r 0 g 255 b 0 - transparent
    for row in range(np_array.shape[0]):
        for col in range(np_array.shape[1]):
            color: np.ndarray = np_array[row][col][:3]
            color >>= 3

            for i, color_ in enumerate(palette):
                color_: np.ndarray
                if all(color == color_):
                    break
            else:
                palette.append(color)
                i = len(palette) - 1

            np_array_palette[row][col] = i  # color 0 reserved for transparent

    palette = palette[1:]  # remove transparent color

    color8bit = True
    if len(palette) <= 15 and not FORCE_8BIT:
        color8bit = False
        # Convert array palette from 8bit to 4bit
        np_array_palette2 = np.zeros((np_array.shape[0], (np_array.shape[1] + 1) // 2))
        # print(np_array_palette2.shape)
        np_array_palette2[:] = np_array_palette[:, ::2]
        if np_array.shape[1] % 2 == 0:
            np_array_palette2[:] += np_array_palette[:, 1::2] << 4
        else:
            np_array_palette2[:, :-1] += np_array_palette[:, 1::2] << 4
        np_array_palette = np_array_palette2

    def get_tile(tile_x, tile_y):
        size_ = 8 if color8bit else 4
        tile_ = np.zeros((8, size_), np.uint8)
        for y in range(8):
            if tile_y * 8 + y >= np_array.shape[0]:
                break
            copy_length = min(size_, np_array_palette.shape[1] - tile_x*size_)
            tile_[y][:copy_length] = np_array_palette[tile_y*8+y][tile_x*size_:tile_x*size_+copy_length]
        return tile_

    tiles = []

    tile_map = np.zeros(((np_array.shape[0] + 7) // 8, (np_array.shape[1] + 7) // 8),
                        dtype=np.dtype(np.uint16).newbyteorder("<"))

    for tile_row in range((np_array.shape[0] + 7) // 8):
        for tile_col in range((np_array.shape[1] + 7) // 8):
            tile = get_tile(tile_col, tile_row)

            for i, t in enumerate(tiles):
                if all((t == tile).flatten()):
                    break
            else:
                tiles.append(tile)
                i = len(tiles) - 1

            tile_map[tile_row][tile_col] = i

    palette = np.array([c[0] + (c[1] << 5) + (c[2] << 10) for c in palette],
                       dtype=np.dtype(np.uint16).newbyteorder("<"))
    tiles = np.array(tiles)

    wtr = binary.BinaryWriter(open(output_file, "wb"))
    wtr.write(b"CBGF")
    file_size_pos = wtr.tell()
    wtr.write_uint32(0)
    wtr.write_uint32(1)  # Version
    wtr.write_uint8(1 if color8bit else 0)

    # begin palette
    wtr.write_uint8(len(palette))
    wtr.write(palette.tobytes())

    # begin tiles
    wtr.write_uint16(len(tiles))
    wtr.write(tiles.tobytes())

    # begin map
    wtr.write_uint8(tile_map.shape[1])
    wtr.write_uint8(tile_map.shape[0])
    wtr.write(tile_map.tobytes())

    size = wtr.tell()
    wtr.seek(file_size_pos)
    wtr.write_uint32(size)
    wtr.close()


def compileBackgrounds():
    for root, _, files in os.walk("bg"):
        for file in files:
            path = os.path.join(root, file)
            path_dest = os.path.splitext(os.path.join("../nitrofs", path))[0] + ".cbgf"
            if os.path.isfile(path_dest):
                src_time = os.path.getmtime(path)
                dst_time = os.path.getmtime(path_dest)
                if src_time > dst_time:
                    convert(path, path_dest)
            else:
                pathlib.Path(os.path.split(path_dest)[0]).mkdir(exist_ok=True, parents=True)
                convert(path, path_dest)


if __name__ == '__main__':
    compileBackgrounds()
