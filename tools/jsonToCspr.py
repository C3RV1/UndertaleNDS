import pathlib

import binary
import numpy as np
from PIL import Image
import json
import os


def get_ones_in_bin(x):
    count = 0
    while x > 0:
        count += x & 1;
        x >>= 1
    return count


# TODO: Rewrite for version 6
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
    # TODO: Sort palette
    palette = np.array([c[0] + (c[1] << 5) + (c[2] << 10) for c in palette],
                       dtype=np.dtype(np.uint16).newbyteorder("<"))

    width, height = data["size"]
    top_down_offset = data.get("top_down_offset", height)
    frame_count = data["frameCount"]

    contain_oam = not data.get("excludeOam", False)
    contain_3d = not data.get("exclude3d", False)
    if contain_oam and len(palette) > 15:
        raise Exception("OAM with more than 15 colors")

    def get_big_tile(frame_, _tile_x, _tile_y, _tile_w, _tile_h):
        tile_ = np.zeros((_tile_h * 8, _tile_w * 8), np.uint8)
        for y in range(_tile_h * 8):
            if _tile_y * 8 + y >= height:
                break
            copy_length = max(0, min(_tile_w * 8, width - _tile_x*8))
            tile_[y][:copy_length] = np_array_palette[_tile_y*8 + frame_ * height + y][_tile_x*8:_tile_x*8+copy_length]
        if len(palette) < 16:
            tile_4_bit = np.zeros((_tile_h * 8, _tile_w * 4), np.uint8)
            for y in range(_tile_h * 8):
                for x in range(_tile_w * 8):
                    tile_4_bit[y][x // 2] += tile_[y][x] << ((x & 1) * 4)
            tile_ = tile_4_bit
        return tile_

    def get_tile(frame_, _tile_x, _tile_y):
        return get_big_tile(frame_, _tile_x, _tile_y, 1, 1)


    wtr = binary.BinaryWriter(open(output_file, "wb"))
    wtr.write(b"CSPR")
    file_size_pos = wtr.tell()
    wtr.write_uint32(0)
    wtr.write_uint32(6)  # Version
    wtr.write_uint16(width)
    wtr.write_uint16(height)
    wtr.write_uint16(top_down_offset)
    wtr.write_uint8(frame_count)
    wtr.write_bool(contain_oam)
    wtr.write_bool(contain_3d)

    wtr.write_uint8(len(palette))
    wtr.write(palette.tobytes())

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

    tile_w, tile_h = (width + 7) // 8, (height + 7) // 8

    if contain_oam:
        oam_w = (tile_w + 7) // 8
        oam_h = (tile_h + 7) // 8
        wtr.write_uint8(oam_w)
        wtr.write_uint8(oam_h)
        for oam_y in range(oam_h):
            for oam_x in range(oam_w):
                reserve_x = 8
                reserve_y = 8
                if oam_x == oam_w - 1:
                    reserve_x = tile_w - (oam_w - 1) * 8
                if oam_y == oam_h - 1:
                    reserve_y = tile_h - (oam_h - 1) * 8
                if reserve_x > 4:
                    reserve_x = 8
                elif reserve_x > 2:
                    reserve_x = 4
                if reserve_y > 4:
                    reserve_y = 8
                elif reserve_y > 2:
                    reserve_y = 4
                if reserve_x == 8 and reserve_y < 8:
                    reserve_y = 4
                if reserve_y == 8 and reserve_x < 8:
                    reserve_x = 4
                wtr.write_uint8(reserve_x)
                wtr.write_uint8(reserve_y)
                for frame in range(frame_count):
                    for tile_y in range(oam_y*8, oam_y*8 + reserve_y):
                        for tile_x in range(oam_x*8, oam_x*8 + reserve_x):
                            wtr.write(get_tile(frame, tile_x, tile_y).tobytes())

    if contain_3d:
        alloc_x = get_ones_in_bin(tile_w)
        alloc_y = get_ones_in_bin(tile_h)
        wtr.write_uint8(alloc_x)
        wtr.write_uint8(alloc_y)

        tile_y = 0
        tile_h_ = tile_h
        while tile_h_ > 0:
            sub_tile_h = 1
            while (sub_tile_h << 1) <= tile_h_:
                sub_tile_h <<= 1

            tile_x = 0
            tile_w_ = tile_w
            while tile_w_ > 0:
                sub_tile_w = 1
                while (sub_tile_w << 1) <= tile_w_:
                    sub_tile_w <<= 1

                print(sub_tile_w, sub_tile_h)
                wtr.write_uint8(sub_tile_w)
                wtr.write_uint8(sub_tile_h)
                for frame in range(frame_count):
                    wtr.write(get_big_tile(frame, tile_x, tile_y, sub_tile_w, sub_tile_h).tobytes())

                tile_w_ -= sub_tile_w
                tile_x += sub_tile_w
            tile_h_ -= sub_tile_h
            tile_y += sub_tile_h

    size = wtr.tell()
    print(size, file_size_pos)
    wtr.seek(file_size_pos)
    wtr.write_uint32(size)
    wtr.close()


def compile_sprites():
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
    compile_sprites()
