import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.start_dialogue(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 1),
                     "leftIdle", "leftTalk",
                     type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.start_bgm("mus_tension.wav", True)
    c.player_control(True)
    c.set_animation(Target(TargetType.SPRITE, 1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 1), 1180 - 90, 0,
                     frames_from_dst(1180 - 90, 120))
    c.wait(WaitTypes.FRAMES, frames_from_dst(1180 - 90, 120))
    c.set_pos(Target(TargetType.SPRITE, 1), 1179, 37)
    c.set_action(Target(TargetType.SPRITE, 1), "cutscene", 1)
