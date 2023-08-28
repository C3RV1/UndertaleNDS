import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_collider_enabled(0, False)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.set_collider_enabled(0, False)
    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
