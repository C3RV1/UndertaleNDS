import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.set_collider_enabled(0, False)
    c.play_sfx("snd_phone.wav", 0)
    c.start_dialogue(10, "", 0, 0, "", "",
                     Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.start_dialogue(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.start_dialogue(30, "", 0, 0, "", "",
                     Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.set_flag(FlagOffsets.PROGRESS, 9)
