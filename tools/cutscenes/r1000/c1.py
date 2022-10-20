import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    # == SAY HELLO CELL ==
    c.player_control(False)
    c.play_sfx("snd_phone.wav", 0)
    c.start_dialogue(10, "", 0, 0, "", "",
                     Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.cmp_flag(FlagOffsets.TORIEL_HELLO_COUNT, "==", 0)
    next_call = c.jump_if_not()

    c.start_dialogue(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.set_flag(FlagOffsets.TORIEL_HELLO_COUNT, 1)
    skip_next_call = c.jump()

    c.bind(next_call)
    c.start_dialogue(21, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.bind(skip_next_call)
    c.start_dialogue(30, "", 0, 0, "", "",
                     Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
