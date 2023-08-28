import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.cmp_flag(FlagOffsets.PROGRESS, "==", 3)
    toriel_not_in_room = c.jump_if_not()

    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "worriedLeftIdle", "worriedLeftTalk",
                        Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    jump_end = c.jump()

    c.bind(toriel_not_in_room)

    c.dialogue_centered(11, "", 0, 0, "", "", Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.bind(jump_end)
