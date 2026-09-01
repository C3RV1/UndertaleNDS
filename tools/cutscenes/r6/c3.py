import typing

if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (
        Cutscene,
        Target,
        TargetType,
        SpriteIDs,
        PLAYER_SPEED,
        frames_from_dst,
        WaitTypes,
    )
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(4, False)
    c.set_action(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "cutscene", 4)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        638 - 612,
        0,
        frames_from_dst(638 - 612, PLAYER_SPEED),
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(638 - 612, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        0,
        103 - 127,
        frames_from_dst(103 - 127, PLAYER_SPEED),
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(103 - 127, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        764 - 638,
        0,
        frames_from_dst(764 - 638, PLAYER_SPEED),
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(764 - 638, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightIdle")
