import enum
from typing import List

import binary


def to_fixed_point(f: float):
    return int(f * (2 ** 8))


PLAYER_SPEED = 90


def frames_from_dst(dst, px_per_sec):
    return (abs(dst) * 60) // px_per_sec


class CutsceneCommands(enum.IntEnum):
    LOAD_SPRITE = 0  # Done
    PLAYER_CONTROL = 1  # Done
    WAIT = 2  # Done
    SET_SHOWN = 3  # Done
    SET_ANIMATION = 4  # Done
    SET_POS = 5  # Done
    MOVE_IN_FRAMES = 6  # Done
    START_DIALOGUE = 7  # Done
    START_BATTLE = 8  # Little done
    EXIT_BATTLE = 9  # Done
    # LOAD_TEXTURE = 10  # Done
    BATTLE_ATTACK = 11  # Done (requires implementing attacks)
    BATTLE_ACTION = 12
    # CHECK_HIT = 13  # Done
    JUMP_IF = 14  # Done
    JUMP_IF_NOT = 15  # Done
    JUMP = 16  # Done
    MANUAL_CAMERA = 17  # Done
    UNLOAD_SPRITE = 18  # Done
    SCALE_IN_FRAMES = 19  # Done
    SET_SCALE = 20  # Done
    START_BGM = 21  # Done
    STOP_BGM = 22  # Done
    SET_POS_IN_FRAMES = 23  # Done
    SET_FLAG = 24  # Done
    CMP_FLAG = 25  # Done
    SET_COLLIDER_ENABLED = 26  # Done
    # UNLOAD_TEXTURE = 27  # Done
    SET_ACTION = 28  # Done
    PLAY_SFX = 29  # Done
    SAVE_MENU = 30  # Done
    MAX_HEALTH = 31  # Done
    MOD_FLAG = 32  # Done
    CMP_ENEMY_HP = 33  # Done
    # SET_ENEMY_ATTACK = 34  # Done
    # SET_ENEMY_ACT = 35
    CLEAR_NAV_TASKS = 36  # Done
    LOAD_SPRITE_RELATIVE = 37  # Done
    SET_CELL = 38  # Done
    MOVE = 39  # Done
    SET_OPACITY = 40  # Done
    CLEAR = 41  # Done
    ENEMY_COMMAND = 42  # Done
    DEBUG = 0xff  # Done


class WaitTypes(enum.IntEnum):
    NONE = 0  # Done
    FRAMES = 1  # Done
    EXIT = 2  # Done
    ENTER = 3  # Done
    DIALOGUE = 4  # Done
    BATTLE_ATTACK = 5  # Done
    SAVE_MENU = 6  # Done
    BATTLE_ACTION = 7


class FlagOffsets(enum.IntEnum):
    PROGRESS = 0
    DUMMY = 1
    OWNS_PHONE = 2
    TORIEL_HELLO_COUNT = 3
    TORIEL_FLIRT_COUNT = 4
    TORIEL_CALL_MOM = 5
    ROOM_FLAGS = 210
    BATTLE_FLAGS = 220
    BATTLE_ACTION = 230
    DIALOGUE_OPTION = 231
    PERSISTENT = 240
    CINNAMON_BUTTERSCOTCH = 241


class BattleAttackIds(enum.IntEnum):
    NONE = 0
    MOVEMENT_TUTORIAL = 1
    FLOWEY_ATTACK = 2
    FLOWEY_ATTACK2 = 3


class TargetType(enum.IntEnum):
    NULL = 0
    PLAYER = 1
    SPRITE = 2
    CAMERA = 3
    ENEMY = 4


class ComparisonOperator(enum.IntEnum):
    EQUALS = 0
    GREATER_THAN = 1
    LESS_THAN = 2


class BtlActionOff(enum.IntEnum):
    FIGHT = 0
    ACT = 10
    FLEE = 40
    SPARE = 41
    ITEMS = 60


class EnemyID(enum.IntEnum):
    FLOWEY = 0
    DUMMY = 1


class FloweyCommands(enum.IntEnum):
    MOVEMENT = 0
    PROGRESS_TO_ATTACK = 1
    PROGRESS_TO_KILL = 2


class DialogueTypes(enum.IntEnum):
    LEFT_ALIGNED = 0
    CENTERED = 1
    FLAVOR_TEXT = 2


class Target:
    def __init__(self, target_type: int, target_id: int = 0,
                 enemy_sprite_id: int = 0):
        self.target_type: int = target_type
        self.target_id: int = target_id
        self.enemy_sprite_id: int = enemy_sprite_id

    def write(self, wtr):
        wtr.write_uint8(self.target_type)
        if self.target_type == TargetType.SPRITE:
            wtr.write_int8(self.target_id)
        elif self.target_type == TargetType.ENEMY:
            wtr.write_int8(self.target_id)
            wtr.write_uint8(self.enemy_sprite_id)


class Cutscene:
    def __init__(self, wtr: binary.BinaryWriter):
        self.wtr: binary.BinaryWriter = wtr
        self.version = 14
        self.file_size_pos = 0
        self.instructions_address = []
        self.pending_address = {}
        self.init_cutscene()

    def init_cutscene(self):
        self.wtr.write(b"CSCN")
        self.wtr.write_uint32(self.version)
        self.file_size_pos = self.wtr.tell()
        self.wtr.write_uint32(0)

    def end_cutscene(self):
        if len(self.pending_address) > 0:
            raise RuntimeError(f"Missing jump bind on instructions "
                               f"{', '.join([str(k) for k in self.pending_address.keys()])}")
        size = self.wtr.tell()
        self.wtr.seek(self.file_size_pos)
        self.wtr.write_uint32(size)
        self.wtr.seek(size)
        self.wtr.close()

    def write_header(self, command_idx):
        self.instructions_address.append(self.wtr.tell())
        self.wtr.write_uint8(command_idx)

    def debug(self, string: str):
        self.write_header(CutsceneCommands.DEBUG)
        self.wtr.write_string(string, encoding="ascii")
        return self.instructions_address[-1]

    def load_sprite(self, x: float, y: float, texture: str, layer=1):
        self.write_header(CutsceneCommands.LOAD_SPRITE)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_int32(layer)
        self.wtr.write_string(texture, encoding="ascii")
        return self.instructions_address[-1]

    def load_sprite_relative(self, dx: float, dy: float, texture: str,
                             target: Target, layer=1):
        self.write_header(CutsceneCommands.LOAD_SPRITE_RELATIVE)
        self.wtr.write_int32(to_fixed_point(dx))
        self.wtr.write_int32(to_fixed_point(dy))
        self.wtr.write_int32(layer)
        self.wtr.write_string(texture, encoding="ascii")
        target.write(self.wtr)
        return self.instructions_address[-1]

    def unload_sprite(self, sprite_id: int):
        self.write_header(CutsceneCommands.UNLOAD_SPRITE)
        self.wtr.write_int8(sprite_id)
        return self.instructions_address[-1]

    def player_control(self, control: bool):
        self.write_header(CutsceneCommands.PLAYER_CONTROL)
        self.wtr.write_bool(control)
        return self.instructions_address[-1]

    def manual_camera(self, control: bool):
        self.write_header(CutsceneCommands.MANUAL_CAMERA)
        self.wtr.write_bool(control)
        return self.instructions_address[-1]

    def set_collider_enabled(self, collider_id: int, enabled: bool):
        self.write_header(CutsceneCommands.SET_COLLIDER_ENABLED)
        self.wtr.write_uint8(collider_id)
        self.wtr.write_bool(enabled)
        return self.instructions_address[-1]

    def wait(self, wait_type: WaitTypes, value: int = 0):
        self.write_header(CutsceneCommands.WAIT)
        self.wtr.write_uint8(wait_type)
        if wait_type == WaitTypes.FRAMES:
            self.wtr.write_uint16(value)
        return self.instructions_address[-1]

    def set_shown(self, target: Target, shown: bool):
        self.write_header(CutsceneCommands.SET_SHOWN)
        target.write(self.wtr)
        self.wtr.write_bool(shown)
        return self.instructions_address[-1]

    def set_animation(self, target: Target, animation: str):
        self.write_header(CutsceneCommands.SET_ANIMATION)
        target.write(self.wtr)
        self.wtr.write_string(animation, encoding="ascii")
        return self.instructions_address[-1]
    
    def set_opacity(self, target: Target, opacity: int):
        self.write_header(CutsceneCommands.SET_OPACITY)
        target.write(self.wtr)
        self.wtr.write_uint8(opacity)
        return self.instructions_address[-1]

    # == NAVIGATION ==
    def set_pos(self, target: Target, x: float, y: float):
        self.write_header(CutsceneCommands.SET_POS)
        target.write(self.wtr)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        return self.instructions_address[-1]

    def set_scale(self, target: Target, x: float, y: float):
        self.write_header(CutsceneCommands.SET_SCALE)
        target.write(self.wtr)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        return self.instructions_address[-1]

    def set_pos_in_frames(self, target: Target, x: float, y: float, frames: int):
        self.write_header(CutsceneCommands.SET_POS_IN_FRAMES)
        target.write(self.wtr)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_uint16(frames)
        return self.instructions_address[-1]

    def move_in_frames(self, target: Target, dx: float, dy: float, frames: int):
        self.write_header(CutsceneCommands.MOVE_IN_FRAMES)
        target.write(self.wtr)
        dx = to_fixed_point(abs(dx)) * (1 if dx > 0 else -1)
        dy = to_fixed_point(abs(dy)) * (1 if dy > 0 else -1)
        self.wtr.write_int32(dx)
        self.wtr.write_int32(dy)
        self.wtr.write_uint16(frames)
        return self.instructions_address[-1]

    def move(self, target: Target, dx: float, dy: float):
        self.write_header(CutsceneCommands.MOVE)
        target.write(self.wtr)
        dx = to_fixed_point(abs(dx)) * (1 if dx > 0 else -1)
        dy = to_fixed_point(abs(dy)) * (1 if dy > 0 else -1)
        self.wtr.write_int32(dx)
        self.wtr.write_int32(dy)
        return self.instructions_address[-1]

    def scale_in_frames(self, target: Target, x: float, y: float, frames: int):
        self.write_header(CutsceneCommands.SCALE_IN_FRAMES)
        target.write(self.wtr)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_uint16(frames)
        return self.instructions_address[-1]

    def clear_nav_tasks(self):
        self.write_header(CutsceneCommands.CLEAR_NAV_TASKS)
        return self.instructions_address[-1]

    def set_action(self, target: Target, action: str, cutscene_id=0):
        self.write_header(CutsceneCommands.SET_ACTION)
        target.write(self.wtr)
        action_int = {
            "none": 0,
            "cutscene": 1
        }[action]
        self.wtr.write_uint8(action_int)
        if action_int == 1:
            self.wtr.write_uint16(cutscene_id)
        return self.instructions_address[-1]

    # == DIALOGUE ==
    def dialogue_centered(self, dialogue_text_id: int,
                          speaker_path: str, x: float, y: float,
                          idle_anim: str, talk_anim: str,
                          speaker_target: Target,
                          idle_anim2: str, talk_anim2: str,
                          type_sound: str = "",
                          font: str = "fnt_maintext.font", frames_per_letter=3,
                          main_screen=False):
        self.write_header(CutsceneCommands.START_DIALOGUE)
        self.wtr.write_uint8(DialogueTypes.CENTERED)
        self.wtr.write_uint16(dialogue_text_id)
        self.wtr.write_string(speaker_path, encoding="ascii")
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_string(idle_anim, encoding="ascii")
        self.wtr.write_string(talk_anim, encoding="ascii")
        speaker_target.write(self.wtr)
        self.wtr.write_string(idle_anim2, encoding="ascii")
        self.wtr.write_string(talk_anim2, encoding="ascii")
        self.wtr.write_string(type_sound, encoding="ascii")
        self.wtr.write_string(font, encoding="ascii")
        self.wtr.write_uint16(frames_per_letter)
        self.wtr.write_bool(main_screen)
        return self.instructions_address[-1]

    def dialogue_left_align(self, dialogue_text_id: int,
                            x: float, y: float,
                            speaker_target: Target,
                            idle_anim: str, talk_anim: str, type_sound: str = "",
                            font: str = "fnt_maintext.font", frames_per_letter=2,
                            main_screen=False):
        self.write_header(CutsceneCommands.START_DIALOGUE)
        self.wtr.write_uint8(DialogueTypes.LEFT_ALIGNED)
        self.wtr.write_uint16(dialogue_text_id)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        speaker_target.write(self.wtr)
        self.wtr.write_string(idle_anim, encoding="ascii")
        self.wtr.write_string(talk_anim, encoding="ascii")
        self.wtr.write_string(type_sound, encoding="ascii")
        self.wtr.write_string(font, encoding="ascii")
        self.wtr.write_uint16(frames_per_letter)
        self.wtr.write_bool(main_screen)
        return self.instructions_address[-1]
    
    def dialogue_flavor(self, dialogue_text_id: int,
                        type_sound: str = "", font: str = "fnt_maintext.font", frames_per_letter: int = 2):
        self.write_header(CutsceneCommands.START_DIALOGUE)
        self.wtr.write_uint8(DialogueTypes.FLAVOR_TEXT)
        self.wtr.write_uint16(dialogue_text_id)
        self.wtr.write_string(type_sound, encoding="ascii")
        self.wtr.write_string(font, encoding="ascii")
        self.wtr.write_uint16(frames_per_letter)
        return self.instructions_address[-1]

    
    def clear(self, main_screen: bool = False):
        self.write_header(CutsceneCommands.CLEAR)
        self.wtr.write_bool(main_screen)
        return self.instructions_address[-1]

    # == BATTLE ==
    def start_battle(self, enemy_ids: List[int], board_id: int,
                     board_x: int, board_y: int, board_w: int, board_h: int,
                     battle_background: str = "battle/battle_bg_simple",
                     start_with_flavor = True):
        self.write_header(CutsceneCommands.START_BATTLE)
        self.wtr.write_uint8(len(enemy_ids))
        for enemy in enemy_ids:
            self.wtr.write_uint8(enemy)
        self.wtr.write_uint8(board_id)
        self.wtr.write_uint8(board_x)
        self.wtr.write_uint8(board_y)
        self.wtr.write_uint8(board_w)
        self.wtr.write_uint8(board_h)
        self.wtr.write_bool(start_with_flavor)
        self.wtr.write_string(battle_background, encoding="ascii")
        return self.instructions_address[-1]

    def exit_battle(self, won=False):
        self.write_header(CutsceneCommands.EXIT_BATTLE)
        self.wtr.write_bool(won)
        return self.instructions_address[-1]

    def start_battle_attacks(self):
        self.write_header(CutsceneCommands.BATTLE_ATTACK)
        return self.instructions_address[-1]

    def battle_action(self, flavor_text_id):
        self.write_header(CutsceneCommands.BATTLE_ACTION)
        self.wtr.write_int16(flavor_text_id)
        return self.instructions_address[-1]

    def enemy_command(self, enemy_idx, enemy_cmd):
        self.write_header(CutsceneCommands.ENEMY_COMMAND)
        self.wtr.write_uint8(enemy_idx)
        self.wtr.write_uint8(enemy_cmd)
        return self.instructions_address[-1]


    # == SAVE ==
    def set_flag(self, flag_id: int, flag_value: int):
        self.write_header(CutsceneCommands.SET_FLAG)
        self.wtr.write_uint16(flag_id)
        self.wtr.write_uint16(flag_value)
        return self.instructions_address[-1]

    def mod_flag(self, flag_id: int, flag_mod: int):
        self.write_header(CutsceneCommands.MOD_FLAG)
        self.wtr.write_uint16(flag_id)
        self.wtr.write_int16(flag_mod)
        return self.instructions_address[-1]

    def set_cell(self, phone_options: List[int]):
        self.write_header(CutsceneCommands.SET_CELL)
        for option in phone_options[:8]:
            if option == 0:
                continue
            self.wtr.write_uint8(option)
        self.wtr.write_uint8(0)
        return self.instructions_address[-1]

    def cmp_flag(self, flag_id: int, operator: str, value: int):
        self.write_header(CutsceneCommands.CMP_FLAG)
        self.wtr.write_uint16(flag_id)
        op_byte = {
            "==": 0,
            "!=": 0,
            ">": 1,
            "<=": 1,
            "<": 2,
            ">=": 2
        }[operator]
        op_byte += (1 << 2) if operator in ["!=", "<=", ">="] else 0
        self.wtr.write_uint8(op_byte)
        self.wtr.write_uint16(value)
        return self.instructions_address[-1]

    def cmp_enemy_hp(self, enemy_id: int, operator: str, value: int):
        self.write_header(CutsceneCommands.CMP_ENEMY_HP)
        self.wtr.write_uint8(enemy_id)
        op_byte = {
            "==": 0,
            "!=": 0,
            ">": 1,
            "<=": 1,
            "<": 2,
            ">=": 2
        }[operator]
        op_byte += (1 << 2) if operator in ["!=", "<=", ">="] else 0
        self.wtr.write_uint8(op_byte)
        self.wtr.write_uint16(value)
        return self.instructions_address[-1]

    # == LOGIC ==
    def jump_if(self, dst=None):
        self.write_header(CutsceneCommands.JUMP_IF)
        if dst is None:
            self.pending_address[self.instructions_address[-1]] = self.wtr.tell()
            self.wtr.write_uint32(0)
        else:
            self.wtr.write_uint32(dst)
        return self.instructions_address[-1]

    def jump_if_not(self, dst=None):
        self.write_header(CutsceneCommands.JUMP_IF_NOT)
        if dst is None:
            self.pending_address[self.instructions_address[-1]] = self.wtr.tell()
            self.wtr.write_uint32(0)
        else:
            self.wtr.write_uint32(dst)
        return self.instructions_address[-1]

    def jump(self, dst=None):
        self.write_header(CutsceneCommands.JUMP)
        if dst is None:
            self.pending_address[self.instructions_address[-1]] = self.wtr.tell()
            self.wtr.write_uint32(0)
        else:
            self.wtr.write_uint32(dst)
        return self.instructions_address[-1]

    def bind(self, jump_id):
        if jump_id in self.pending_address:
            pos = self.wtr.tell()
            self.wtr.seek(self.pending_address[jump_id])
            self.wtr.write_uint32(pos)
            self.wtr.seek(pos)
            del self.pending_address[jump_id]
        else:
            raise RuntimeError(f"Error binding jump {jump_id}. Does not exist.")

    # == MUSIC ==
    def start_bgm(self, path: str, loop: bool):
        self.write_header(CutsceneCommands.START_BGM)
        self.wtr.write_bool(loop)
        self.wtr.write_string(path, encoding="ascii")
        return self.instructions_address[-1]

    def stop_bgm(self):
        self.write_header(CutsceneCommands.STOP_BGM)
        return self.instructions_address[-1]

    def play_sfx(self, path: str, loops: int = 0):
        self.write_header(CutsceneCommands.PLAY_SFX)
        self.wtr.write_uint8(loops)
        self.wtr.write_string(path, encoding="ascii")
        return self.instructions_address[-1]

    def save_menu(self):
        self.write_header(CutsceneCommands.SAVE_MENU)
        return self.instructions_address[-1]

    def max_health(self):
        self.write_header(CutsceneCommands.MAX_HEALTH)
        return self.instructions_address[-1]

