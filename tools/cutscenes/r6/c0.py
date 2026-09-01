import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, WaitTypes, SpriteIDs
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_collider_enabled(1, False)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.dialogue_centered(
        10,
        "speaker/toriel",
        (256 - 50) // 2,
        (192 - 39) // 4,
        "talkIdle",
        "talkTalk",
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        "downIdle",
        "downTalk",
        type_sound="snd_txttor.wav",
    )
    c.wait(WaitTypes.DIALOGUE)
