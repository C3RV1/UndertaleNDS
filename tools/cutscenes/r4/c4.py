import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (Cutscene, Target, TargetType, WaitTypes,
        FlagOffsets, SpriteIDs)
else:
    from CutsceneTypes import *


PRESSED_LEVER_ID = 2
SPIKE_IDS = [3, 4]


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.SPRITE, PRESSED_LEVER_ID), "down")
    c.set_action(Target(TargetType.SPRITE, PRESSED_LEVER_ID), "none")
    c.set_collider_enabled(30, False)

    # TODO: Screen shake
    c.play_sfx("snd_screenshake.wav", 0)
    for i in SPIKE_IDS:
        c.set_animation(Target(TargetType.SPRITE, i), "down")

    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "slightSmileIdle", "slightSmileTalk",
                        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.player_control(True)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 700 - 640, 0, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.unload_sprite(SpriteIDs.TORIEL_ROOM)

    c.set_flag(FlagOffsets.PROGRESS, 4)
