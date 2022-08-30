import enum
import binary


class CutsceneCommands(enum.IntEnum):
    LOAD_SPRITE = 0
    PLAYER_CONTROL = 1
    WAIT_LOAD = 2
    SHOW = 3
    HIDE = 4
    SET_ANIMATION = 5
    WAIT_FRAMES = 6
    SET_POS = 7
    MOVE_IN_FRAMES = 8
    START_DIALOGUE = 9
    WAIT_DIALOGUE_END = 10
    START_BATTLE = 11
    EXIT_BATTLE = 12
    START_BATTLE_DIALOGUE = 13
    BATTLE_ATTACK = 14
    WAIT_BATTLE_ATTACK = 15
    WAIT_BATTLE_ACTION = 16
    CMP_BATTLE_ACTION = 17
    CHECK_HIT = 18
    JUMP_IF = 19
    JUMP_IF_NOT = 20
    JUMP = 21
    MANUAL_CAMERA = 22
    UNLOAD_SPRITE = 23
    SCALE_IN_FRAMES = 24
    SET_SCALE = 25
    DEBUG = 0xff


class TargetType(enum.IntEnum):
    NULL = 0
    PLAYER = 1
    SPRITE = 2
    CAMERA = 3


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
        self.version = 1
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
            raise RuntimeError(f"Trying to jump to instructions {', '.join([k for k in self.pending_address.keys()])}, "
                               f"instructions do not exist.")
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

    def load_sprite(self, x: int, y: int, sprite_path: str):
        self.write_header(CutsceneCommands.LOAD_SPRITE)
        self.wtr.write_uint16(x)
        self.wtr.write_uint16(y)
        self.wtr.write_string(sprite_path, encoding="ascii")

    def unload_sprite(self, room_sprite_id: int):
        self.write_header(CutsceneCommands.UNLOAD_SPRITE)
        self.wtr.write_uint8(room_sprite_id)

    def player_control(self, control: bool):
        self.write_header(CutsceneCommands.PLAYER_CONTROL)
        self.wtr.write_bool(control)

    def manual_camera(self, control: bool):
        self.write_header(CutsceneCommands.MANUAL_CAMERA)
        self.wtr.write_bool(control)

    def wait_load(self):
        self.write_header(CutsceneCommands.WAIT_LOAD)

    def show(self, target: Target):
        self.write_header(CutsceneCommands.SHOW)
        target.write(self.wtr)

    def hide(self, target: Target):
        self.write_header(CutsceneCommands.HIDE)
        target.write(self.wtr)

    def set_animation(self, target: Target, animation: str):
        self.write_header(CutsceneCommands.SET_ANIMATION)
        target.write(self.wtr)
        self.wtr.write_string(animation, encoding="ascii")

    def wait_frames(self, frames: int):
        self.write_header(CutsceneCommands.WAIT_FRAMES)
        self.wtr.write_uint16(frames)

    # == NAVIGATION ==
    def set_pos(self, target: Target, x: int, y: int):
        self.write_header(CutsceneCommands.SET_POS)
        target.write(self.wtr)
        self.wtr.write_uint16(x)
        self.wtr.write_uint16(y)

    def set_scale(self, target: Target, x: int, y: int):
        self.write_header(CutsceneCommands.SET_SCALE)
        target.write(self.wtr)
        self.wtr.write_uint16(x)
        self.wtr.write_uint16(y)

    def move_in_frames(self, target: Target, x: int, y: int, frames: int):
        self.write_header(CutsceneCommands.MOVE_IN_FRAMES)
        target.write(self.wtr)
        self.wtr.write_uint16(x)
        self.wtr.write_uint16(y)
        self.wtr.write_uint16(frames)

    def scale_in_frames(self, target: Target, x: int, y: int, frames: int):
        self.write_header(CutsceneCommands.SCALE_IN_FRAMES)
        target.write(self.wtr)
        self.wtr.write_uint16(x)
        self.wtr.write_uint16(y)
        self.wtr.write_uint16(frames)

    # == DIALOGUE ==
    def start_dialogue(self, dialogue_text_id: int,
                       speaker_path: str, x: int, y: int,
                       idle_anim: str, talk_anim: str,
                       speaker_target: Target,
                       idle_anim2: str, talk_anim2: str):
        self.write_header(CutsceneCommands.START_DIALOGUE)
        self.wtr.write_uint16(dialogue_text_id)
        self.wtr.write_string(speaker_path, encoding="ascii")
        self.wtr.write_uint8(x)
        self.wtr.write_uint8(y)
        self.wtr.write_string(idle_anim, encoding="ascii")
        self.wtr.write_string(talk_anim, encoding="ascii")
        speaker_target.write(self.wtr)
        self.wtr.write_string(idle_anim2, encoding="ascii")
        self.wtr.write_string(talk_anim2, encoding="ascii")

    def wait_dialogue_end(self):
        self.write_header(CutsceneCommands.WAIT_DIALOGUE_END)

    # == BATTLE ==
    def start_battle(self):
        self.write_header(CutsceneCommands.START_BATTLE)

    def exit_battle(self):
        self.write_header(CutsceneCommands.EXIT_BATTLE)

    def start_battle_dialogue(self, x, y, dialogue_text_id,
                              speaker_target: Target, idle_anim: str,
                              talk_anim: str, duration: int):
        self.write_header(CutsceneCommands.START_BATTLE_DIALOGUE)
        self.wtr.write_uint8(x)
        self.wtr.write_uint8(y)
        self.wtr.write_uint16(dialogue_text_id)
        speaker_target.write(self.wtr)
        self.wtr.write_string(idle_anim, encoding="ascii")
        self.wtr.write_string(talk_anim, encoding="ascii")
        self.wtr.write_uint16(duration)

    def battle_attack(self, attack_pattern_id):
        self.write_header(CutsceneCommands.BATTLE_ATTACK)
        self.wtr.write_uint16(attack_pattern_id)

    def wait_battle_attack(self):
        self.write_header(CutsceneCommands.WAIT_BATTLE_ATTACK)

    def wait_battle_action(self, act_actions):
        self.write_header(CutsceneCommands.WAIT_BATTLE_ACTION)
        self.wtr.write_uint8(len(act_actions))
        for act_action in act_actions:
            self.wtr.write_string(act_action, encoding="ascii")

    def cmp_battle_action(self, compare_action):
        self.write_header(CutsceneCommands.CMP_BATTLE_ACTION)
        self.wtr.write_uint8(compare_action)

    def check_hit(self):
        self.write_header(CutsceneCommands.CHECK_HIT)

    # == LOGIC ==
    def jump_if(self):
        self.write_header(CutsceneCommands.JUMP_IF)
        self.pending_address[len(self.instructions_address) - 1] = self.wtr.tell()
        self.wtr.write_uint32(0)
        return len(self.instructions_address) - 1

    def jump_if_not(self):
        self.write_header(CutsceneCommands.JUMP_IF_NOT)
        self.pending_address[len(self.instructions_address) - 1] = self.wtr.tell()
        self.wtr.write_uint32(0)
        return len(self.instructions_address) - 1

    def jump(self):
        self.write_header(CutsceneCommands.JUMP)
        self.pending_address[len(self.instructions_address) - 1] = self.wtr.tell()
        self.wtr.write_uint32(0)
        return len(self.instructions_address) - 1

    def bound_jump(self, jump_id):
        if jump_id in self.pending_address:
            pos = self.wtr.tell()
            self.wtr.seek(self.pending_address[jump_id])
            self.wtr.write_uint32(pos)
            self.wtr.seek(pos)
            del self.pending_address[jump_id]
