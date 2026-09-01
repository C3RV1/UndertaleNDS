import typing

if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (
        Cutscene,
        Target,
        TargetType,
        frames_from_dst,
        PLAYER_SPEED,
        WaitTypes,
        SpriteIDs,
    )
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(2, False)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        455 - 155,
        0,
        frames_from_dst(455 - 155, PLAYER_SPEED),
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(455 - 155, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        0,
        127 - 86,
        frames_from_dst(127 - 86, PLAYER_SPEED),
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(127 - 86, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        635 - 459,
        0,
        frames_from_dst(635 - 459, PLAYER_SPEED),
    )
    # Do not wait, exit cutscene right away to allow new cutscene
    # c.wait(WaitTypes.FRAMES, frames_from_dst(635 - 459, 70))
