import pathlib
from typing import List

import binary
import json
import os


def to_fixed_point(f: float):
    return int(f * (2 ** 8))


class RoomHeader:
    def __init__(self):
        self.header = b"ROOM"
        self.file_size_pos = 0
        self.version = 9

    def write(self, wtr: binary.BinaryWriter):
        wtr.write(self.header)
        self.file_size_pos = wtr.tell()
        wtr.write_uint32(0)  # placeholder file size
        wtr.write_uint32(self.version)

    def write_size(self, wtr: binary.BinaryWriter):
        size = wtr.tell()
        wtr.seek(self.file_size_pos)
        wtr.write_uint32(size)
        wtr.seek(size)


class RoomExit:
    def __init__(self):
        self.exit_type = 0
        self.room_id = 0
        self.spawn_y = 0
        self.spawn_x = 0
        self.x = 0
        self.y = 0
        self.w = 0
        self.h = 0
        self.side = 0

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint8(self.exit_type)
        wtr.write_uint16(self.room_id)
        wtr.write_uint16(self.spawn_x)
        wtr.write_uint16(self.spawn_y)
        if self.exit_type == 0:
            wtr.write_uint8(self.side)
        elif self.exit_type == 1:
            wtr.write_uint16(self.x)
            wtr.write_uint16(self.y)
            wtr.write_uint16(self.w)
            wtr.write_uint16(self.h)

    @classmethod
    def from_dict(cls, dct):
        res = cls()
        res.exit_type = {
            "side": 0,
            "rect": 1
        }[dct["exit_type"]]
        res.room_id = dct["room_id"]
        res.spawn_x = dct["spawn_x"]
        res.spawn_y = dct["spawn_y"]
        if res.exit_type == 0:
            res.side = {
                "up": 0,
                "down": 1,
                "left": 2,
                "right": 3
            }[dct["side"]]
        elif res.exit_type == 1:
            res.x = dct["x"]
            res.y = dct["y"]
            res.w = dct["w"]
            res.h = dct["h"]
        return res


class RoomExits:
    def __init__(self):
        self.exits: List[RoomExit] = []

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint8(len(self.exits))
        for ext in self.exits:
            ext.write(wtr)

    @classmethod
    def from_list(cls, lst):
        res = cls()
        for element in lst:
            res.exits.append(RoomExit.from_dict(element))
        return res


class RoomTextures:
    def __init__(self):
        self.textures = []

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint8(len(self.textures))
        for texture in self.textures:
            wtr.write_string(texture, encoding="ascii")

    @classmethod
    def from_list(cls, lst):
        res = cls()
        res.textures = lst
        return res


class RoomSprite:
    def __init__(self):
        self.texture_id = 0
        self.x = 0
        self.y = 0
        self.animation = ""
        self.action = 0
        self.cutscene_id = 0
        self.distance = 0
        self.close_anim = ""
        self.parallax_x = 0
        self.parallax_y = 0

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint8(self.texture_id)
        wtr.write_uint16(int(self.x))
        wtr.write_uint16(int(self.y))
        wtr.write_string(self.animation, encoding="ascii")
        wtr.write_uint8(self.action)
        if self.action == 1:
            wtr.write_uint16(self.cutscene_id)
        elif self.action == 2:
            wtr.write_uint16(self.distance)
            wtr.write_string(self.close_anim, encoding="ascii")
        elif self.action == 3:
            wtr.write_int32(to_fixed_point(self.parallax_x))
            wtr.write_int32(to_fixed_point(self.parallax_y))

    @classmethod
    def from_dict(cls, dct):
        res = cls()
        res.texture_id = dct["texture_id"]
        res.x = dct["x"]
        res.y = dct["y"]
        res.animation = dct.get("animation", "gfx")
        res.action = {
            "none": 0,
            "cutscene": 1,
            "proximity": 2,
            "parallax": 3
        }[dct.get("action", "none")]
        if res.action == 1:
            res.cutscene_id = dct["cutscene_id"]
        elif res.action == 2:
            res.distance = dct["distance"]
            res.close_anim = dct["close_animation"]
        elif res.action == 3:
            res.parallax_x = dct.get("parallax_x", 1.0)
            res.parallax_y = dct.get("parallax_y", 1.0)
            res.x -= 256//2
            res.y -= 192//2
            if res.parallax_x != 0:
                res.x *= res.parallax_x
            if res.parallax_y != 0:
                res.y *= res.parallax_y
            res.x += 256//2
            res.y += 192//2
        return res


class RoomSprites:
    def __init__(self):
        self.sprites: List[RoomSprite] = []

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint8(len(self.sprites))
        for sprite in self.sprites:
            sprite.write(wtr)

    @classmethod
    def from_list(cls, lst):
        res = cls()
        for element in lst:
            res.sprites.append(RoomSprite.from_dict(element))
        return res


class RoomCollider:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.w = 0
        self.h = 0
        self.collider_action = 0
        self.enabled = True
        self.cutscene_id = 0

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint16(self.x)
        wtr.write_uint16(self.y)
        wtr.write_uint16(self.w)
        wtr.write_uint16(self.h)
        wtr.write_uint8(self.collider_action)
        wtr.write_bool(self.enabled)
        if self.collider_action == 1:
            wtr.write_uint16(self.cutscene_id)

    @classmethod
    def from_dict(cls, dct: dict):
        res = cls()
        res.x = dct["x"]
        res.y = dct["y"]
        res.w = dct["w"]
        res.h = dct["h"]
        res.collider_action = {
            "wall": 0,
            "cutscene": 1
        }[dct.get("action", "wall")]
        res.enabled = dct.get("enabled", True)
        if res.collider_action == 1:
            res.cutscene_id = dct["cutscene_id"]
        return res


class RoomColliders:
    def __init__(self):
        self.colliders: List[RoomCollider] = []

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint16(len(self.colliders))
        for collider in self.colliders:
            collider.write(wtr)

    @classmethod
    def from_list(cls, lst):
        res = cls()
        for element in lst:
            res.colliders.append(RoomCollider.from_dict(element))
        return res


class RoomPartCondition:
    def __init__(self):
        self.flag_id = 0
        self.cmp_operator = 0
        self.cmp_value = 0

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint16(self.flag_id)
        wtr.write_uint8(self.cmp_operator)
        wtr.write_uint16(self.cmp_value)

    @classmethod
    def from_dict(cls, dct):
        res = cls()
        res.flag_id = dct["flag_id"]
        res.cmp_operator = {
            "==": 0,
            "!=": 0,
            ">": 1,
            "<=": 1,
            "<": 2,
            ">=": 2
        }[dct["op"]]
        res.cmp_operator += (1 << 2) if dct["op"] in ["!=", "<=", ">="] else 0  # add flip bit
        res.cmp_value = dct["cmp_value"]
        return res


class RoomPart:
    def __init__(self):
        self.conditions: List[RoomPartCondition] = []
        self.room_bg = ""
        self.music_path = ""
        self.music_volume = 127
        self.spawn_x = 0
        self.spawn_y = 0
        self.room_exits = RoomExits()
        self.room_textures = RoomTextures()
        self.room_sprites = RoomSprites()
        self.room_colliders = RoomColliders()

    def write(self, wtr: binary.BinaryWriter):
        start_pos = wtr.tell()
        wtr.write_uint32(0)
        wtr.write_uint8(len(self.conditions))
        for condition in self.conditions:
            condition.write(wtr)
        wtr.write_string(self.room_bg, encoding="ascii")
        wtr.write_string(self.music_path, encoding="ascii")
        wtr.write_uint8(self.music_volume)
        wtr.write_uint16(self.spawn_x)
        wtr.write_uint16(self.spawn_y)
        self.room_exits.write(wtr)
        self.room_textures.write(wtr)
        self.room_sprites.write(wtr)
        self.room_colliders.write(wtr)
        end_pos = wtr.tell()
        wtr.seek(start_pos)
        wtr.write_uint32(end_pos - start_pos - 4)
        wtr.seek(end_pos)

    @classmethod
    def from_dict(cls, dct: dict):
        res = cls()
        for condition in dct["conditions"]:
            res.conditions.append(RoomPartCondition.from_dict(condition))
        res.room_bg = dct["room_bg"]
        res.music_path = dct["music_path"]
        res.music_volume = dct.get("music_volume", 127)
        res.spawn_x = dct.get("spawn_x")
        res.spawn_y = dct.get("spawn_y")
        res.room_exits = RoomExits.from_list(dct["exits"])
        res.room_textures = RoomTextures.from_list(dct["textures"])
        res.room_sprites = RoomSprites.from_list(dct["sprites"])
        res.room_colliders = RoomColliders.from_list(dct["colliders"])
        return res


class RoomFile:
    def __init__(self):
        self.header = RoomHeader()
        self.parts: List[RoomPart] = []

    def write(self, wtr: binary.BinaryWriter):
        self.header.write(wtr)
        wtr.write_uint8(len(self.parts))
        for part in self.parts:
            part.write(wtr)
        self.header.write_size(wtr)

    @classmethod
    def from_dict(cls, dct):
        res = cls()
        for part in dct["parts"]:
            res.parts.append(RoomPart.from_dict(part))
        return res


def convert(input_file, output_file):
    print(f"Converting {input_file} to {output_file}")
    with open(input_file, "r") as f:
        json_data = json.loads(f.read())
    room_file = RoomFile.from_dict(json_data)
    wtr = binary.BinaryWriter(open(output_file, "wb"))
    room_file.write(wtr)
    wtr.close()


def compile_rooms():
    for root, _, files in os.walk("rooms"):
        for file in files:
            path = os.path.join(root, file)
            path_dest = os.path.splitext(os.path.join("../nitrofs/data", path))[0] + ".room"
            if os.path.isfile(path_dest):
                src_time = os.path.getmtime(path)
                dst_time = os.path.getmtime(path_dest)
                if src_time > dst_time:
                    convert(path, path_dest)
            else:
                pathlib.Path(os.path.split(path_dest)[0]).mkdir(exist_ok=True, parents=True)
                convert(path, path_dest)


if __name__ == '__main__':
    compile_rooms()
