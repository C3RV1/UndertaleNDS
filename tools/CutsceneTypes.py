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
    LOAD_TEXTURE = 10  # Done
    BATTLE_ATTACK = 11  # Done (requires implementing attacks)
    BATTLE_ACTION = 12
    CHECK_HIT = 13  # Done
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
    UNLOAD_TEXTURE = 27  # Done
    SET_ACTION = 28  # Done
    PLAY_SFX = 29  # Done
    SAVE_MENU = 30  # Done
    MAX_HEALTH = 31  # Done
    MOD_FLAG = 32  # Done
    CMP_ENEMY_HP = 33  # Done
    SET_ENEMY_ATTACK = 34  # Done
    SET_ENEMY_ACT = 35
    CLEAR_NAV_TASKS = 36  # Done
    LOAD_SPRITE_RELATIVE = 37  # Done
    SET_CELL = 38  # Done
    MOVE = 39
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
    # BATTLE_HIT = 231 ? (replacing check_hit?)
    PERSISTENT = 240


class BattleAttackIds(enum.IntEnum):
    NONE = 0
    MOVEMENT_TUTORIAL = 1
    FLOWEY_ATTACK = 2
    FLOWEY_ATTACK2 = 3


class Enemy:
    def __init__(self, id_, hp, defense, act_text_id, act_option_count, attack_id,
                 spare_value=0, gold_on_win=0, exp_on_kill=0):
        self.id_ = id_
        self.hp = hp
        self.act_text_id = act_text_id
        self.act_option_count = act_option_count
        self.attack_id = attack_id
        self.spare_value = spare_value
        self.gold_on_win = gold_on_win
        self.exp_on_kill = exp_on_kill
        self.defense = defense

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint16(self.id_)
        wtr.write_uint16(self.hp)
        wtr.write_uint16(self.attack_id)
        wtr.write_uint16(self.act_text_id)
        wtr.write_uint8(self.act_option_count)
        wtr.write_uint8(self.spare_value)
        wtr.write_uint8(self.gold_on_win)
        wtr.write_uint8(self.exp_on_kill)
        wtr.write_int16(self.defense)


class TargetType(enum.IntEnum):
    NULL = 0
    PLAYER = 1
    SPRITE = 2
    CAMERA = 3


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


class Target:
    def __init__(self, target_type: int, target_id: int = 0):
        self.target_type: int = target_type
        self.target_id: int = target_id

    def write(self, wtr):
        wtr.write_uint8(self.target_type)
        if self.target_type == TargetType.SPRITE:
            wtr.write_int8(self.target_id)


class Cutscene:
    def __init__(self, wtr: binary.BinaryWriter):
        self.wtr: binary.BinaryWriter = wtr
        self.version = 8
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

    def load_texture(self, path: str):
        self.write_header(CutsceneCommands.LOAD_TEXTURE)
        self.wtr.write_string(path, encoding="ascii")
        return self.instructions_address[-1]

    def unload_texture(self, texture_id: int):
        self.write_header(CutsceneCommands.UNLOAD_TEXTURE)
        self.wtr.write_int8(texture_id)
        return self.instructions_address[-1]

    def load_sprite(self, x: float, y: float, tex_id: int, layer=1):
        self.write_header(CutsceneCommands.LOAD_SPRITE)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_int32(layer)
        self.wtr.write_int8(tex_id)
        return self.instructions_address[-1]

    def load_sprite_relative(self, dx: float, dy: float, tex_id: int,
                             target: Target, layer=1):
        self.write_header(CutsceneCommands.LOAD_SPRITE_RELATIVE)
        self.wtr.write_int32(to_fixed_point(dx))
        self.wtr.write_int32(to_fixed_point(dy))
        self.wtr.write_int32(layer)
        self.wtr.write_int8(tex_id)
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
        action = {
            "none": 0,
            "cutscene": 1
        }[action]
        self.wtr.write_uint8(action)
        if action == 1:
            self.wtr.write_uint16(cutscene_id)

    # == DIALOGUE ==
    def start_dialogue(self, dialogue_text_id: int,
                       speaker_path: str, x: float, y: float,
                       idle_anim: str, talk_anim: str,
                       speaker_target: Target,
                       idle_anim2: str, talk_anim2: str,
                       type_sound: str = "",
                       font: str = "fnt_maintext.font", frames_per_letter=3,
                       main_screen=False, centered=True):
        self.write_header(CutsceneCommands.START_DIALOGUE)
        self.wtr.write_bool(centered)
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

    def start_dialogue_battle(self, dialogue_text_id: int,
                              x: float, y: float,
                              speaker_target: Target,
                              idle_anim: str, talk_anim: str, type_sound: str = "",
                              font: str = "fnt_maintext.font", frames_per_letter=2,
                              main_screen=False, centered=False):
        return self.start_dialogue(dialogue_text_id, "", x, y,
                                   "", "", speaker_target, idle_anim, talk_anim,
                                   type_sound=type_sound,
                                   font=font, frames_per_letter=frames_per_letter,
                                   main_screen=main_screen, centered=centered)

    # == BATTLE ==
    def start_battle(self, enemies: List[Enemy], board_id: int,
                     board_x: int, board_y: int, board_w: int, board_h: int):
        self.write_header(CutsceneCommands.START_BATTLE)
        self.wtr.write_uint8(len(enemies))
        for enemy in enemies:
            enemy.write(self.wtr)
        self.wtr.write_uint8(board_id)
        self.wtr.write_uint8(board_x)
        self.wtr.write_uint8(board_y)
        self.wtr.write_uint8(board_w)
        self.wtr.write_uint8(board_h)
        return self.instructions_address[-1]

    def exit_battle(self, won=False):
        self.write_header(CutsceneCommands.EXIT_BATTLE)
        self.wtr.write_bool(won)
        return self.instructions_address[-1]

    def start_battle_attacks(self):
        self.write_header(CutsceneCommands.BATTLE_ATTACK)
        return self.instructions_address[-1]

    def battle_action(self):
        self.write_header(CutsceneCommands.BATTLE_ACTION)
        return self.instructions_address[-1]

    def check_hit(self):
        self.write_header(CutsceneCommands.CHECK_HIT)
        return self.instructions_address[-1]

    def set_enemy_attack(self, enemy_idx, attack_id):
        self.write_header(CutsceneCommands.SET_ENEMY_ATTACK)
        self.wtr.write_uint8(enemy_idx)
        self.wtr.write_uint16(attack_id)
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
