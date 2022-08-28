from typing import List

import binary
import json
import sys
import os


class RoomHeader:
    def __init__(self):
        self.header = b"ROOM"
        self.file_size_pos = 0
        self.version = 3

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


class RoomSprite:
    def __init__(self):
        self.sprite_path = ""
        self.x = 0
        self.y = 0
        self.layer = 0
        self.animation = ""
        self.can_interact = False
        self.interact_action = 0
        self.cutscene_id = 0

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_string(self.sprite_path, encoding="ascii")
        wtr.write_uint16(self.x)
        wtr.write_uint16(self.y)
        wtr.write_uint16(self.layer)
        wtr.write_string(self.animation, encoding="ascii")
        wtr.write_bool(self.can_interact)
        wtr.write_uint8(self.interact_action)
        if self.interact_action == 1:
            wtr.write_uint16(self.cutscene_id)

    @classmethod
    def from_dict(cls, dct):
        res = cls()
        res.sprite_path = dct["sprite_path"]
        res.x = dct["x"]
        res.y = dct["y"]
        res.layer = dct.get("layer", 1)
        res.animation = dct["animation"]
        res.can_interact = dct["can_interact"]
        if res.can_interact:
            res.interact_action = dct["interact_action"]
            if res.interact_action == 1:
                res.cutscene_id = dct["cutscene_id"]
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
        self.cutscene_id = 0

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint16(self.x)
        wtr.write_uint16(self.y)
        wtr.write_uint16(self.w)
        wtr.write_uint16(self.h)
        wtr.write_uint8(self.collider_action)
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
            "trigger": 1
        }[dct.get("collider_action", "wall")]
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
        pass

    def write(self, wtr: binary.BinaryWriter):
        pass

    @classmethod
    def from_dict(cls, _dct):
        res = cls()
        return res


class RoomPart:
    def __init__(self):
        self.conditions: List[RoomPartCondition] = []
        self.room_bg = ""
        self.music_path = ""
        self.room_exits = RoomExits()
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
        self.room_exits.write(wtr)
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
        res.room_exits = RoomExits.from_list(dct["exits"])
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
    with open(input_file, "r") as f:
        json_data = json.loads(f.read())
    room_file = RoomFile.from_dict(json_data)
    wtr = binary.BinaryWriter(open(output_file, "wb"))
    room_file.write(wtr)
    wtr.close()


def main():
    if len(sys.argv) != 2:  # name, input
        return
    input_file = sys.argv[1]
    if os.path.isdir(input_file):
        input_file = [os.path.join(input_file, fp) for fp in os.listdir(input_file) if fp.endswith(".json")]
    else:
        input_file = [input_file]
    for fp in input_file:
        convert(fp, os.path.splitext(fp)[0] + ".room")


if __name__ == '__main__':
    main()
