import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.SPRITE, 2), "down")
    c.set_action(Target(TargetType.SPRITE, 2), "none")
    c.set_collider_enabled(2, False)

    # TODO: Screen shake
    c.play_sfx("snd_screenshake.wav", 0)
    c.set_animation(Target(TargetType.SPRITE, 5), "down")
    c.set_animation(Target(TargetType.SPRITE, 6), "down")

    c.start_dialogue(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "slightSmileIdle", "slightSmileTalk",
                     Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.player_control(True)

    c.set_animation(Target(TargetType.SPRITE, 7), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 7), 700 - 640, 0, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.unload_sprite(7)
    c.unload_texture(4)

    c.set_flag(FlagOffsets.PROGRESS, 4)
