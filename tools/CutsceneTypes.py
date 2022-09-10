import enum
from typing import List

import binary


def to_fixed_point(f: float):
    return int(f * (2 ** 8))


class CutsceneCommands(enum.IntEnum):
    LOAD_SPRITE = 0  # Done
    PLAYER_CONTROL = 1  # Done
    WAIT_EXIT = 2  # Done
    WAIT_ENTER = 3  # Done
    SET_SHOWN = 4  # Done
    SET_ANIMATION = 5  # Done
    WAIT_FRAMES = 6  # Done
    SET_POS = 7  # Done
    MOVE_IN_FRAMES = 8  # Done
    START_DIALOGUE = 9  # Done
    WAIT_DIALOGUE_END = 10  # Done
    START_BATTLE = 11  # Little done
    EXIT_BATTLE = 12
    LOAD_TEXTURE = 13
    BATTLE_ATTACK = 14
    WAIT_BATTLE_ATTACK = 15
    WAIT_BATTLE_ACTION = 16
    CMP_BATTLE_ACTION = 17
    CHECK_HIT = 18  # Done
    JUMP_IF = 19  # Done
    JUMP_IF_NOT = 20  # Done
    JUMP = 21  # Done
    MANUAL_CAMERA = 22  # Done
    UNLOAD_SPRITE = 23  # Done
    SCALE_IN_FRAMES = 24  # Done
    SET_SCALE = 25  # Done
    START_BGM = 26  # Done
    STOP_BGM = 27  # Done
    SET_POS_IN_FRAMES = 28  # Done
    SET_FLAG = 29  # Done
    CMP_FLAG = 30  # Done
    SET_COLLIDER_ENABLED = 31  # Done
    UNLOAD_TEXTURE = 32
    DEBUG = 0xff  # Done


class Enemy:
    def __init__(self, enemy_id, enemy_hp):
        self.id_ = enemy_id
        self.hp = enemy_hp

    def write(self, wtr: binary.BinaryWriter):
        wtr.write_uint16(self.id_)
        wtr.write_uint16(self.hp)


class TargetType(enum.IntEnum):
    NULL = 0
    PLAYER = 1
    SPRITE = 2
    CAMERA = 3


class ComparisonOperator(enum.IntEnum):
    EQUALS = 0
    GREATER_THAN = 1
    LESS_THAN = 2


class SaveFlags(enum.IntEnum):
    RUINS_PROGRESS = 0


class Target:
    def __init__(self, target_type: int, target_id: int = 0):
        self.target_type: int = target_type
        self.target_id: int = target_id

    def write(self, wtr):
        wtr.write_uint8(self.target_type)
        if self.target_type == TargetType.SPRITE:
            wtr.write_uint8(self.target_id)


class Cutscene:
    def __init__(self, wtr: binary.BinaryWriter):
        self.wtr: binary.BinaryWriter = wtr
        self.version = 3
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
        self.wtr.write_uint8(texture_id)
        return self.instructions_address[-1]

    def load_sprite(self, x: float, y: float, tex_id: int, layer=1):
        self.write_header(CutsceneCommands.LOAD_SPRITE)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_int32(layer)
        self.wtr.write_uint8(tex_id)
        return self.instructions_address[-1]

    def unload_sprite(self, sprite_id: int):
        self.write_header(CutsceneCommands.UNLOAD_SPRITE)
        self.wtr.write_uint8(sprite_id)
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

    def wait_exit(self):
        self.write_header(CutsceneCommands.WAIT_EXIT)
        return self.instructions_address[-1]

    def wait_enter(self):
        self.write_header(CutsceneCommands.WAIT_ENTER)
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

    def wait_frames(self, frames: int):
        self.write_header(CutsceneCommands.WAIT_FRAMES)
        self.wtr.write_uint16(frames)
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

    def scale_in_frames(self, target: Target, x: float, y: float, frames: int):
        self.write_header(CutsceneCommands.SCALE_IN_FRAMES)
        target.write(self.wtr)
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_uint16(frames)
        return self.instructions_address[-1]

    # == DIALOGUE ==
    def start_dialogue(self, dialogue_text_id: int,
                       speaker_path: str, x: float, y: float,
                       idle_anim: str, talk_anim: str,
                       speaker_target: Target,
                       idle_anim2: str, talk_anim2: str,
                       font: str, frames_per_letter=3):
        self.write_header(CutsceneCommands.START_DIALOGUE)
        self.wtr.write_uint16(dialogue_text_id)
        self.wtr.write_string(speaker_path, encoding="ascii")
        self.wtr.write_int32(to_fixed_point(x))
        self.wtr.write_int32(to_fixed_point(y))
        self.wtr.write_string(idle_anim, encoding="ascii")
        self.wtr.write_string(talk_anim, encoding="ascii")
        speaker_target.write(self.wtr)
        self.wtr.write_string(idle_anim2, encoding="ascii")
        self.wtr.write_string(talk_anim2, encoding="ascii")
        self.wtr.write_string(font, encoding="ascii")
        self.wtr.write_uint16(frames_per_letter)
        return self.instructions_address[-1]

    def start_dialogue_battle(self, dialogue_text_id: int,
                              x: float, y: float,
                              speaker_target: Target,
                              idle_anim: str, talk_anim: str,
                              font: str, frames_per_letter=2):
        return self.start_dialogue(dialogue_text_id, "", x, y,
                                   "", "", speaker_target, idle_anim, talk_anim,
                                   font, frames_per_letter=frames_per_letter)

    def wait_dialogue_end(self):
        self.write_header(CutsceneCommands.WAIT_DIALOGUE_END)
        return self.instructions_address[-1]

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

    def exit_battle(self):
        self.write_header(CutsceneCommands.EXIT_BATTLE)
        return self.instructions_address[-1]

    def battle_attack(self, attack_pattern_id):
        self.write_header(CutsceneCommands.BATTLE_ATTACK)
        self.wtr.write_uint16(attack_pattern_id)
        return self.instructions_address[-1]

    def wait_battle_attack(self):
        self.write_header(CutsceneCommands.WAIT_BATTLE_ATTACK)
        return self.instructions_address[-1]

    def wait_battle_action(self, text_id, act_actions):
        self.write_header(CutsceneCommands.WAIT_BATTLE_ACTION)
        self.wtr.write_uint8(text_id)
        self.wtr.write_uint8(len(act_actions))
        for act_action in act_actions:
            self.wtr.write_uint8(act_action)
        return self.instructions_address[-1]

    def cmp_battle_action(self, compare_action):
        self.write_header(CutsceneCommands.CMP_BATTLE_ACTION)
        self.wtr.write_uint8(compare_action)
        return self.instructions_address[-1]

    def check_hit(self):
        self.write_header(CutsceneCommands.CHECK_HIT)
        return self.instructions_address[-1]

    # == SAVE ==
    def set_flag(self, flag_id: int, flag_value: int):
        self.write_header(CutsceneCommands.SET_FLAG)
        self.wtr.write_uint16(flag_id)
        self.wtr.write_uint16(flag_value)
        return self.instructions_address[-1]

    def cmp_flag(self, flag_id: int, operator: int, value: int):
        self.write_header(CutsceneCommands.CMP_FLAG)
        self.wtr.write_uint16(flag_id)
        self.wtr.write_uint8(operator)
        self.wtr.write_uint16(value)
        return self.instructions_address[-1]

    # == LOGIC ==
    def jump_if(self):
        self.write_header(CutsceneCommands.JUMP_IF)
        self.pending_address[self.instructions_address[-1]] = self.wtr.tell()
        self.wtr.write_uint32(0)
        return self.instructions_address[-1]

    def jump_if_not(self):
        self.write_header(CutsceneCommands.JUMP_IF_NOT)
        self.pending_address[self.instructions_address[-1]] = self.wtr.tell()
        self.wtr.write_uint32(0)
        return self.instructions_address[-1]

    def jump(self):
        self.write_header(CutsceneCommands.JUMP)
        self.pending_address[self.instructions_address[-1]] = self.wtr.tell()
        self.wtr.write_uint32(0)
        return self.instructions_address[-1]

    def bind(self, jump_id, dst=None):
        if jump_id in self.pending_address:
            pos = self.wtr.tell()
            self.wtr.seek(self.pending_address[jump_id])
            if dst is None:
                self.wtr.write_uint32(pos)
            else:
                self.wtr.write_uint32(dst)
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
