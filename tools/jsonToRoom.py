from __future__ import annotations
import traceback
from typing import Any, Optional
from collections.abc import Callable, Sequence
from spriteIDs import ROOM_SPRITE_NAMES_TO_SPRITE_IDS
import binary
import json
import os
import pathlib
import enum

def tuple_unpacker[T](val_unpacker: Callable[[Any], T], expected_length: int) -> Callable[[Any], tuple]:
    def inner_unpacker(x: tuple):
        if len(x) != expected_length:
            raise ValueError(f"Tuple should be {expected_length} elements long.")
        return tuple([val_unpacker(v) for v in x])
    return inner_unpacker

def tuple_packer(val_packer: Callable[[Any], None]) -> Callable[[tuple], None]:
    def inner_packer(x: tuple):
        for v in x:
            val_packer(v)
    return inner_packer

def int_unpacker(x: Any) -> int:
    if not isinstance(x, int):
        raise ValueError(f"{repr(x)} is not an int.")
    return x

def float_unpacker(x: Any) -> int:
    if not isinstance(x, float):
        raise ValueError(f"{repr(x)} is not a float.")
    return x

def bool_unpacker(x: Any) -> bool:
    if not isinstance(x, bool):
        raise ValueError(f"{repr(x)} is not a bool.")
    return x

def str_unpacker(x: Any) -> str:
    if not isinstance(x, str):
        raise ValueError(f"{repr(x)} is not a string.")
    return x

def to_fixed_point(f: float) -> int:
    return int(f * (2 ** 8))

def fixed_point_wtr(wtr: binary.BinaryWriter) -> Callable[[float], None]:
    def inner_fixed_point_wtr(f: float):
        wtr.write_int32(to_fixed_point(f))
    return inner_fixed_point_wtr

def str_wtr(wtr: binary.BinaryWriter) -> Callable[[str], None]:
    def inner_string_wtr(string: str):
        wtr.write_string(string, encoding="ascii")
    return inner_string_wtr

def obj_wtr(cw: ConditionalWriter) -> Callable[[Obj], None]:
    def inner_obj_wtr(obj: Obj):
        obj.write(cw)
    return inner_obj_wtr

class RoomHeader:
    def __init__(self) -> None:
        self.header = b"ROOM"
        self.file_size_pos = 0
        self.version = 12

    def write(self, wtr: binary.BinaryWriter) -> None:
        wtr.write(self.header)
        self.file_size_pos = wtr.tell()
        wtr.write_uint32(0)  # placeholder file size
        wtr.write_uint32(self.version)

    def write_size(self, wtr: binary.BinaryWriter) -> None:
        size = wtr.tell()
        wtr.seek(self.file_size_pos)
        wtr.write_uint32(size)
        wtr.seek(size)

class ConditionalWriter:
    def __init__(self, wtr: binary.BinaryWriter) -> None:
        self.wtr = wtr
        self.last_non_condition_count = 0
        self.last_non_condition_pos: Optional[int] = None

    def write_last_unconditional_count(self) -> None:
        pos = self.wtr.tell()

        if self.last_non_condition_pos is None:
            raise RuntimeError("Trying to write last unconditional without pos.")

        self.wtr.seek(self.last_non_condition_pos)
        self.wtr.write_uint8(self.last_non_condition_count - 1)
        self.wtr.seek(pos)

        self.last_non_condition_count = 0

    def start_unconditional_data(self) -> None:
        if self.last_non_condition_count == 0xFF:
            self.write_last_unconditional_count()
        if self.last_non_condition_count == 0:
            self.last_non_condition_pos = self.wtr.tell()
            self.wtr.write_uint8(0)
        self.last_non_condition_count += 1
    
    def start_conditional_data(self) -> None:
        if self.last_non_condition_count != 0:
            self.write_last_unconditional_count()
        self.wtr.write_uint8(0xFF)

    def end_write(self) -> None:
        if self.last_non_condition_count != 0:
            self.write_last_unconditional_count()

class ConditionalObj:
    def __init__(self) -> None:
        pass


class Obj(ConditionalObj):
    def write(self, cw: ConditionalWriter) -> None:
        _ = cw
        raise NotImplementedError("Raw obj should not be used.")


class ObjData[T]:
    def write(self, cw: ConditionalWriter, data_wtr: Callable[[T], None]) -> None:
        _ = cw
        _ = data_wtr
        raise NotImplementedError("Use of raw ObjData is incorrect.")

class ObjUnconditionalData[T](ObjData[T]):
    def __init__(self, data: T) -> None:
        super().__init__()
        self.data: T = data

    def write(self, cw: ConditionalWriter,
              data_wtr: Callable[[T], None]) -> None:
        cw.start_unconditional_data()
        data_wtr(self.data)

class ObjCondition:
    FLIP_BIT = 1 << 2
    HAS_NEXT_CONDITION_BIT = 1 << 3
    HAS_NEXT_VARIATION_BIT = 1 << 4
    OR_WITH_PREVIOUS_BIT = 1 << 5
    
    def __init__(self, flag: int, cmp: int | str, cmp_value: int,
                 or_with_prev: bool | str) -> None:
        self.flag: int = flag
        if isinstance(cmp, str):
            cmp = {
                "==": 0,
                "!=": ObjCondition.FLIP_BIT + 0,
                ">": 1,
                "<=": ObjCondition.FLIP_BIT + 1,
                "<": 2,
                ">=": ObjCondition.FLIP_BIT + 2
            }[cmp]
        if isinstance(or_with_prev, str):
            or_with_prev = {
                "and": False,
                "or": True
            }[or_with_prev]

        cmp: int
        self.cmp: int = cmp
        if or_with_prev:
            self.cmp |= ObjCondition.OR_WITH_PREVIOUS_BIT
        self.cmp_value: int = cmp_value

    def write(self, wtr: binary.BinaryWriter, has_next_condition: bool,
              has_next_variation: bool) -> None:
        wtr.write_uint16(self.flag)
        cmp = self.cmp
        if has_next_condition:
            cmp |= ObjCondition.HAS_NEXT_CONDITION_BIT
        elif has_next_variation:
            cmp |= ObjCondition.HAS_NEXT_VARIATION_BIT
        wtr.write_uint8(cmp)
        wtr.write_uint16(self.cmp_value)

    @classmethod
    def from_json_obj(cls, json_obj: dict[str, Any]) -> ObjCondition:
        return cls(json_obj["flag"], json_obj["cmp"], json_obj["val"],
                   json_obj.get("chain", "and"))
        

class ObjConditionAndValue[T](ObjData[T]):
    def __init__(self, conditions: Sequence[ObjCondition], value: T,
                 next: Optional[ObjConditionAndValue[T] | T] = None) -> None:
        super().__init__()
        self.value: T = value
        self.conditions: Sequence[ObjCondition] = conditions
        self.next: Optional[ObjConditionAndValue[T] | T] = next

    def set_next(self, next: ObjConditionAndValue[T] | T) -> None:
        self.next = next

    def write(self, cw: ConditionalWriter, data_wtr: Callable[[T], None]) -> None:
        cw.start_conditional_data()

        if self.next is None:
            raise RuntimeError("Next cannot be None")

        # Note: Should be ObjConditionAndValue[T], but cannot check with
        # generics. Can there be any issue with checking only for
        # ObjConditionAndValue? Probably not, as the next can be or a
        # raw type or a ObjConditionAndValue[T].
        has_next_variation = isinstance(self.next, ObjConditionAndValue)
        for i, cond in enumerate(self.conditions):
            has_next_cond = (i != len(self.conditions) - 1)
            cond.write(cw.wtr, has_next_cond, has_next_variation)

        data_wtr(self.value)

        if isinstance(self.next, ObjConditionAndValue):
            self.next.write(cw, data_wtr)
        else:
            data_wtr(self.next)

def unpack_data[T](json_obj: list[dict[str, Any] | Any] | Any, value_unpacker: Callable[[Any], T]) -> ObjData[T]:
    if not isinstance(json_obj, list) or not isinstance(json_obj[0], dict) or "if" not in json_obj[0]:
        return ObjUnconditionalData(value_unpacker(json_obj))

    root: Optional[ObjConditionAndValue[T]] = None
    last: Optional[ObjConditionAndValue[T]] = None
    for i, v in enumerate(json_obj):
        if not isinstance(v, dict) or v.get("if", None) is None or v.get("then", None) is None:
            if root is None or last is None:
                raise ValueError("Condition list without variations is not allowed.")
            last: ObjConditionAndValue[T]

            if i != len(json_obj) - 1:
                raise ValueError("Raw value in between of conditional list.")

            last.set_next(value_unpacker(v))
            return root

        conditions_list = v["if"]
        if isinstance(conditions_list, list):
            conditions = [ObjCondition.from_json_obj(c) for c in conditions_list]
        else:
            conditions = [ObjCondition.from_json_obj(conditions_list)]

        if len(conditions) == 0:
            raise ValueError("Empty condition list.")

        value = value_unpacker(v["then"])

        node = ObjConditionAndValue(conditions, value, None)
        if root is None or last is None:
            root = node
            last = node
        else:
            last: ObjConditionAndValue[T]
            last.set_next(node)
            last = node

    raise ValueError("Condition list without default last value.")

      
class RoomSideExit(Obj):
    def __init__(
        self,
        room_id: ObjData[int],
        spawn: ObjData[tuple[int, int]],
        exit_type: ObjData[int],
    ) -> None:
        super().__init__()
        self.room_id: ObjData[int] = room_id
        self.spawn: ObjData[tuple[int, int]] = spawn
        self.exit_side: ObjData[int] = exit_type

    def write(self, cw: ConditionalWriter) -> None:
        self.room_id.write(cw, cw.wtr.write_uint16)
        self.spawn.write(cw, tuple_packer(cw.wtr.write_uint16))
        self.exit_side.write(cw, cw.wtr.write_uint8)

    @classmethod
    def from_json_obj(cls, json_obj) -> RoomSideExit:
        def exit_side_unpacker(x: Any) -> int:
            if not isinstance(x, str):
                raise ValueError(f"{repr(x)} is not a valid exit type.")
            return {
                "up": 0,
                "down": 1,
                "left": 2,
                "right": 3
            }[x]

        return cls(
            unpack_data(json_obj["room_id"], int_unpacker),
            unpack_data(json_obj["spawn"], tuple_unpacker(int_unpacker, 2)),
            unpack_data(json_obj["side"], exit_side_unpacker)
        )

class SpriteActionType(enum.IntEnum):
    NONE = 0
    CUTSCENE = 1
    PROXIMITY = 2
    PARALLAX = 3
    PUSHABLE = 4

class RoomSpriteActionUnion(Obj):
    def __init__(self, action_type: SpriteActionType) -> None:
        super().__init__()
        self.type: SpriteActionType = action_type

        # Type 1 = Cutscene
        self.cutscene_id: ObjData[int]= ObjUnconditionalData(0)

        # Type 2 = Proximity
        self.distance: ObjData[int]= ObjUnconditionalData(0)
        self.close_anim: ObjData[str] = ObjUnconditionalData("")

        # Type 3 = Parallax (fixed-point)
        self.parallax: ObjData[tuple[float, float]] = ObjUnconditionalData((0, 0))

        # Type 4 = Pushable
        self.valid_rect: ObjData[tuple[int, int, int, int]] = ObjUnconditionalData(
            (0, 0, 0, 0)
        )
        self.goal_pos: ObjData[tuple[int, int]] = ObjUnconditionalData((0, 0))
        self.cutscene_id: ObjData[int] = ObjUnconditionalData(0)
        self.goal_flag_id: ObjData[int] = ObjUnconditionalData(0)
        self.goal_flag_bit: ObjData[int] = ObjUnconditionalData(0)
        self.stop_on_goal: ObjData[bool] = ObjUnconditionalData(False)

    def write(self, cw: ConditionalWriter) -> None:
        cw.wtr.write_uint8(int(self.type))
        if self.type == SpriteActionType.NONE:
            pass
        elif self.type == SpriteActionType.CUTSCENE:
            self.cutscene_id.write(cw, cw.wtr.write_uint16)
        elif self.type == SpriteActionType.PROXIMITY:
            self.distance.write(cw, cw.wtr.write_uint16)
            self.close_anim.write(cw, str_wtr(cw.wtr))
        elif self.type == SpriteActionType.PARALLAX:
            self.parallax.write(cw, tuple_packer(fixed_point_wtr(cw.wtr)))
        elif self.type == SpriteActionType.PUSHABLE:
            self.valid_rect.write(cw, tuple_packer(cw.wtr.write_uint16))
            self.goal_pos.write(cw, tuple_packer(cw.wtr.write_uint16))
            self.cutscene_id.write(cw, cw.wtr.write_uint16)
            self.goal_flag_id.write(cw, cw.wtr.write_uint16)
            self.goal_flag_bit.write(cw, cw.wtr.write_uint16)
            self.stop_on_goal.write(cw, cw.wtr.write_bool)
        else:
            raise ValueError("Invalid action type.")

    @classmethod
    def from_json_obj(cls, json_obj: dict[str, Any]) -> RoomSpriteActionUnion:
        action_type = {
            "none": SpriteActionType.NONE,
            "cutscene": SpriteActionType.CUTSCENE,
            "proximity": SpriteActionType.PROXIMITY,
            "parallax": SpriteActionType.PARALLAX,
            "pushable": SpriteActionType.PUSHABLE
        }[json_obj.get("action", "none")]
        ret = cls(action_type)
        if action_type == SpriteActionType.CUTSCENE:
            ret.cutscene_id = unpack_data(json_obj["cutscene_id"], int_unpacker)
        elif action_type == SpriteActionType.PROXIMITY:
            ret.distance = unpack_data(json_obj["distance"], int_unpacker)
            ret.close_anim = unpack_data(json_obj["close_animation"],
                                         str_unpacker)
        elif action_type == SpriteActionType.PARALLAX:
            ret.parallax = unpack_data(
                json_obj.get("parallax", (1.0, 1.0)), tuple_unpacker(float_unpacker, 2)
            )
            # TODO: Move this logic to C code
            # res.x -= 256 // 2, res.y -= 192 // 2
            # res.x/y *= res.parallax_x/y if res.parallax_x/y != 0
            # res.x += 256 // 2, res.y += 192 // 2
        elif action_type == SpriteActionType.PUSHABLE:
            ret.valid_rect = unpack_data(
                json_obj["valid_rect"], tuple_unpacker(int_unpacker, 4)
            )
            ret.goal_pos = unpack_data(
                json_obj["goal_pos"], tuple_unpacker(int_unpacker, 2)
            )
            ret.goal_flag_id = unpack_data(json_obj["goal_flag_id"], int_unpacker)
            ret.goal_flag_bit = unpack_data(json_obj["goal_flag_bit"], int_unpacker)
            ret.stop_on_goal = unpack_data(json_obj["stop_on_goal"], bool_unpacker)
        return ret


class RoomSprite(Obj):
    def __init__(
        self,
        spr_id: ObjData[int],
        texture: ObjData[str],
        pos: ObjData[tuple[int, int]],
        animation: ObjData[str],
        action: ObjData[RoomSpriteActionUnion],
    ) -> None:
        super().__init__()
        self.spr_id: ObjData[int] = spr_id
        self.texture: ObjData[str] = texture
        self.pos: ObjData[tuple[int, int]] = pos
        self.animation: ObjData[str] = animation
        self.action: ObjData[RoomSpriteActionUnion] = action

    def write(self, cw: ConditionalWriter) -> None:
        self.spr_id.write(cw, cw.wtr.write_uint16)
        self.texture.write(cw, str_wtr(cw.wtr))
        self.pos.write(cw, tuple_packer(cw.wtr.write_uint16))
        self.animation.write(cw, str_wtr(cw.wtr))
        self.action.write(cw, obj_wtr(cw))
        
    @classmethod
    def from_json_obj(cls, json_obj: dict[str, Any]) -> RoomSprite:
        def spr_id_unpacker(x: Any) -> int:
            if isinstance(x, int):
                if x == 0:
                    raise ValueError("Specified sprite id cannot be 0.")
                return x
            if isinstance(x, str) and x in ROOM_SPRITE_NAMES_TO_SPRITE_IDS:
                return ROOM_SPRITE_NAMES_TO_SPRITE_IDS[x]
            if x is None:
                return 0
            raise ValueError(f"{repr(x)} is not a valid sprite id.")

        return cls(
            unpack_data(json_obj.get("id", None), spr_id_unpacker),
            unpack_data(json_obj["texture"], str_unpacker),
            unpack_data(json_obj["pos"], tuple_unpacker(int_unpacker, 2)),
            unpack_data(json_obj.get("animation", "gfx"), str_unpacker),
            unpack_data(json_obj.get("action", {}), RoomSpriteActionUnion.from_json_obj)
        )

                
class ColliderType(enum.IntEnum):
    WALL = 0
    EXIT = 1
    CUTSCENE = 2

class ColliderTypeUnion(Obj):
    def __init__(self, coll_type: ColliderType) -> None:
        super().__init__()
        self.type: ColliderType = coll_type

        # Type 1 = Exit
        self.room_id: ObjData[int] = ObjUnconditionalData(0)
        self.spawn: ObjData[tuple[int, int]] = ObjUnconditionalData((0, 0))

        # Type 2 = Cutscene
        self.cutscene_id: ObjData[int]= ObjUnconditionalData(0)

    def write(self, cw:ConditionalWriter) -> None:
        cw.wtr.write_uint8(int(self.type))
        if self.type == ColliderType.WALL:
            pass
        elif self.type == ColliderType.EXIT:
            self.room_id.write(cw, cw.wtr.write_uint16)
            self.spawn.write(cw, tuple_packer(cw.wtr.write_uint16))
        elif self.type == ColliderType.CUTSCENE:
            self.cutscene_id.write(cw, cw.wtr.write_uint16)
        else:
            raise ValueError("Invalid collider type.")

    @classmethod
    def from_json_obj(cls, json_obj: dict[str, Any]) -> ColliderTypeUnion:
        coll_type = {
            "wall": ColliderType.WALL,
            "exit": ColliderType.EXIT,
            "cutscene": ColliderType.CUTSCENE,
        }[json_obj.get("type", "wall")]
        ret = cls(coll_type)
        if coll_type == ColliderType.EXIT:
            ret.room_id = unpack_data(json_obj["room_id"], int_unpacker)
            ret.spawn = unpack_data(json_obj["spawn"], tuple_unpacker(int_unpacker, 2))
        elif coll_type == ColliderType.CUTSCENE:
            ret.cutscene_id = unpack_data(json_obj["cutscene_id"], int_unpacker)
        return ret


class RoomCollider(Obj):
    def __init__(self, coll_id: ObjData[int],
                 rect: ObjData[tuple[int, int, int, int]],
                 enabled: ObjData[bool], coll_type: ObjData[ColliderTypeUnion]) -> None:
        super().__init__()
        self.coll_id: ObjData[int] = coll_id
        self.rect: ObjData[tuple[int, int, int, int]] = rect
        self.enabled: ObjData[bool] = enabled
        self.coll_type: ObjData[ColliderTypeUnion] = coll_type

    def write(self, cw: ConditionalWriter) -> None:
        self.coll_id.write(cw, cw.wtr.write_uint8)
        self.rect.write(cw, tuple_packer(cw.wtr.write_uint16))
        self.enabled.write(cw, cw.wtr.write_bool)
        self.coll_type.write(cw, obj_wtr(cw))
        
    @classmethod
    def from_json_obj(cls, json_obj: dict[str, Any]) -> RoomCollider:
        def coll_id_unpacker(x: Any) -> int:
            if x is None:
                return 0
            if not isinstance(x, int):
                raise ValueError(f"{repr(x)} is not a collider id.")
            if x == 0:
                raise ValueError("Specified collider id cannot be 0.")
            return x

        return cls(
            unpack_data(json_obj.get("id", None), coll_id_unpacker),
            unpack_data(json_obj["rect"], tuple_unpacker(int_unpacker, 4)),
            unpack_data(json_obj.get("enabled", True), bool_unpacker),
            unpack_data(json_obj.get("type", {}), ColliderTypeUnion.from_json_obj)
        )


class ListObj[T](ConditionalObj):
    def __init__(self, elements: Sequence[ListObjData[T]]) -> None:
        super().__init__()
        self.elements: Sequence[ListObjData[T]] = elements

    def write(self, cw: ConditionalWriter, data_wtr: Callable[[T], None]) -> None:
        cw.wtr.write_uint16(len(self.elements))
        for v in self.elements:
            v.write(cw, data_wtr)

    @classmethod
    def from_json_list(cls, list: Sequence[Any], value_unpacker: Callable[[Any], T]) -> ListObj[T]:
        elements = []
        for v in list:
            elements.append(unpack_list_data(v, value_unpacker))
        return cls(elements)


class ListObjData[T]:
    def write(self, cw: ConditionalWriter, data_wtr: Callable[[T], None]) -> None:
        pass

class ListObjUnconditionalData[T](ListObjData[T]):
    def __init__(self, data: T) -> None:
        super().__init__()
        self.data: T = data

    def write(self, cw: ConditionalWriter, data_wtr: Callable[[T], None]) -> None:
        cw.start_unconditional_data()
        data_wtr(self.data)


class ListObjConditionalData[T](ListObjData[T]):
    def __init__(self, conditions: Sequence[ObjCondition],
                 data: list[T]) -> None:
        super().__init__()
        self.conditions: Sequence[ObjCondition] = conditions
        self.data: list[T] = data

    def write(self, cw: ConditionalWriter, data_wtr: Callable[[T], None]) -> None:
        if len(self.conditions) == 0:
            raise ValueError("List condition obj without conditions.")

        cw.start_conditional_data()
        for i, v in enumerate(self.conditions):
            has_next_condition = (i != len(self.conditions) - 1)
            v.write(cw.wtr, has_next_condition, False)
        cw.wtr.write_uint8(len(self.data))
        for v in self.data:
            data_wtr(v)

def unpack_list_data[T](json_obj: dict[str, Any | dict[str, Any]] | Any, value_unpacker: Callable[[Any], T]) -> ListObjData[T]:
    if not isinstance(json_obj, dict) or "only_if" not in json_obj:
        return ListObjUnconditionalData(value_unpacker(json_obj))

    condition_list = json_obj["only_if"]
    if isinstance(condition_list, list):
        conditions = [ObjCondition.from_json_obj(c) for c in condition_list]
    elif isinstance(condition_list, dict):
        conditions = [ObjCondition.from_json_obj(condition_list)]
    else:
        raise ValueError("Invalid conditions type.")

    value_list = json_obj["then"]
    if isinstance(value_list, list):
        values = [value_unpacker(c) for c in value_list]
    else:
        values = [value_unpacker(value_list)]
    return ListObjConditionalData(conditions, values)


class Room(Obj):
    def __init__(self, room_bg: ObjData[str], music_path: ObjData[str],
                 music_volume: ObjData[int],
                 spawn: ObjData[tuple[int, int]],
                 room_exits: ListObj[RoomSideExit],
                 room_sprites: ListObj[RoomSprite],
                 room_colliders: ListObj[RoomCollider]
             ) -> None:
        super().__init__()
        self.room_bg: ObjData[str] = room_bg
        self.music_path: ObjData[str] = music_path
        self.music_volume: ObjData[int] = music_volume
        self.spawn: ObjData[tuple[int, int]] = spawn
        self.room_exits: ListObj[RoomSideExit] = room_exits
        self.room_sprites: ListObj[RoomSprite] = room_sprites
        self.room_colliders: ListObj[RoomCollider] = room_colliders

    def write(self, cw: ConditionalWriter) -> None:
        self.room_bg.write(cw, str_wtr(cw.wtr))
        self.music_path.write(cw, str_wtr(cw.wtr))
        self.music_volume.write(cw, cw.wtr.write_uint8)
        self.spawn.write(cw, tuple_packer(cw.wtr.write_uint16))

        self.room_exits.write(cw, obj_wtr(cw))
        self.room_sprites.write(cw, obj_wtr(cw))
        self.room_colliders.write(cw, obj_wtr(cw))

    @classmethod
    def from_json_obj(cls, json_obj: dict[str, Any]) -> Room:
        return cls(
            unpack_data(json_obj["bg"], str_unpacker),
            unpack_data(json_obj["music"], str_unpacker),
            unpack_data(json_obj.get("music_volume", 127), int_unpacker),
            unpack_data(json_obj["spawn"], tuple_unpacker(int_unpacker, 2)),
            ListObj.from_json_list(json_obj["exits"], RoomSideExit.from_json_obj),
            ListObj.from_json_list(json_obj["sprites"], RoomSprite.from_json_obj),
            ListObj.from_json_list(json_obj["colliders"], RoomCollider.from_json_obj)
        )


class RoomFile:
    def __init__(self, room: Room) -> None:
        self.header = RoomHeader()
        self.room: Room = room

    def write(self, wtr: binary.BinaryWriter) -> None:
        self.header.write(wtr)
        cw = ConditionalWriter(wtr)
        self.room.write(cw)
        cw.end_write()
        self.header.write_size(wtr)


def convert(input_file, output_file) -> None:
    print(f"Converting {input_file} to {output_file}")
    with open(input_file, "r") as f:
        json_data = json.loads(f.read())

    try:
        room_file = RoomFile(Room.from_json_obj(json_data))
    except Exception as e:
        print(f"Error converting {input_file}: {repr(e)}")
        traceback.print_exc()
        return
    with open(output_file, "wb") as f:
        wtr = binary.BinaryWriter(f)
        room_file.write(wtr)


def compile_rooms(force: bool = False) -> None:
    for root, _, files in os.walk("rooms"):
        for file in sorted(files):
            path = os.path.join(root, file)
            path_dest = os.path.splitext(os.path.join("../nitrofs", path))[0] + ".room"
            if os.path.isfile(path_dest):
                src_time = os.path.getmtime(path)
                dst_time = os.path.getmtime(path_dest)
                if src_time > dst_time or force:
                    convert(path, path_dest)
            else:
                pathlib.Path(os.path.split(path_dest)[0]).mkdir(exist_ok=True, parents=True)
                convert(path, path_dest)


if __name__ == '__main__':
    compile_rooms(force=True)

