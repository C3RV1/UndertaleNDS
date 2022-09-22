import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.player_control(False)
    c.cmp_flag(220, "==", 1)
    jump_repeated_dialogue = c.jump_if()

    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait_dialogue_end()
    jump_end = c.jump()

    c.bind(jump_repeated_dialogue)
    c.start_dialogue(11, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait_dialogue_end()

    c.bind(jump_end)
    c.set_flag(220, 1)

