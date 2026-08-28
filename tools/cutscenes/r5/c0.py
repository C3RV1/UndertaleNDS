import typing

if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, WaitTypes, SpriteIDs
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.set_collider_enabled(1, False)
    c.dialogue_centered(
        10,
        "speaker/toriel",
        (256 - 50) // 2,
        (192 - 39) // 4,
        "slightSmileIdle",
        "slightSmileTalk",
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        "downIdle",
        "downTalk",
        type_sound="snd_txttor.wav",
    )
    c.wait(WaitTypes.DIALOGUE)

    c.player_control(True)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 136, 41, 40)
    c.wait(WaitTypes.FRAMES, 40)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downIdle")
