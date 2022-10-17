import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_collider_enabled(1, False)
    c.set_collider_enabled(2, False)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.set_animation(Target(TargetType.SPRITE, 1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 1), 50, 0,
                     frames_from_dst(50, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(50, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 1), "downMove")
    c.move_in_frames(Target(TargetType.SPRITE, 1), 0, 40,
                     frames_from_dst(40, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(40, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 1), "leftIdle")

    c.start_bgm("mus_fallendown2.wav", True)
    c.start_dialogue(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 1),
                     "leftIdle", "leftTalk",
                     type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.set_flag(FlagOffsets.OWNS_PHONE, 1)
    c.set_cell([1, 2, 3, 4])

    c.set_animation(Target(TargetType.SPRITE, 1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 1), 100, 0,
                     frames_from_dst(100, 100))
    c.wait(WaitTypes.FRAMES, frames_from_dst(100, 100))

    c.unload_sprite(1)
    c.unload_texture(1)
    c.set_flag(FlagOffsets.PROGRESS, 7)
