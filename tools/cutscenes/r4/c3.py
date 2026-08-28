import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (Cutscene, Target, TargetType, WaitTypes,
        SpriteIDs, FlagOffsets)
else:
    from CutsceneTypes import *


PRESSED_LEVER_ID = 1


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, PRESSED_LEVER_ID), "down")
    c.set_action(Target(TargetType.SPRITE, PRESSED_LEVER_ID), "none")
    c.set_collider_enabled(20, False)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 640-425, 0, 180)
    c.wait(WaitTypes.FRAMES, 180)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "leftIdle")
    c.set_action(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "cutscene",
                 cutscene_id=11)
    c.set_flag(FlagOffsets.ROOM_FLAGS, 0)
