import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.player_control(False)
    c.cmp_flag(0, "==", 3)
    toriel_not_in_room = c.jump_if_not()

    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait_dialogue_end()
    jump_end = c.jump()

    c.bind(toriel_not_in_room)

    c.start_dialogue(11, "", 0, 0, "", "", Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait_dialogue_end()

    c.bind(jump_end)
