import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    # == CALL MOM CELL ==
    c.player_control(False)
    c.play_sfx("snd_phone.wav", 0)
    c.dialogue_centered(10, "", 0, 0, "", "",
                        Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.dialogue_centered(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.dialogue_centered(30, "", 0, 0, "", "",
                        Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.set_flag(FlagOffsets.TORIEL_CALL_MOM, 1)

    # TODO: Remove option from phone
