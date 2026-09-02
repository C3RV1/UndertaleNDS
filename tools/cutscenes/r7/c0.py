import typing

if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (
        Cutscene,
        Target,
        TargetType,
        SpriteIDs,
        WaitTypes,
        frames_from_dst,
    )
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(1, False)
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.dialogue_centered(
        10,
        "speaker/toriel",
        (256 - 50) // 2,
        (192 - 39) // 4,
        "slightSmileIdle",
        "slightSmileTalk",
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        "leftIdle",
        "leftTalk",
        type_sound="snd_txttor.wav",
    )
    c.wait(WaitTypes.DIALOGUE)
    c.start_bgm("mus_tension.wav", True)
    c.player_control(True)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        1180 - 90,
        0,
        frames_from_dst(1180 - 90, 120),
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(1180 - 90, 120))
    c.set_pos(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 1179, 37)
    c.set_action(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "cutscene", 1)
