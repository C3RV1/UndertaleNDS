import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.cmp_flag(FlagOffsets.ROOM_FLAGS, "==", 1)
    jump_repeated_dialogue = c.jump_if()

    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    jump_end = c.jump()

    c.bind(jump_repeated_dialogue)
    c.dialogue_centered(11, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "worriedLeftIdle", "worriedLeftTalk",
                        Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.bind(jump_end)
    c.set_flag(FlagOffsets.ROOM_FLAGS, 1)
