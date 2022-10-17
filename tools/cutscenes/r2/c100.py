import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, WaitTypes
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.play_sfx("snd_heal_c.wav")
    c.max_health()
    c.start_dialogue(10, "", 0, 0, "", "",
                     Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.save_menu()
    c.wait(WaitTypes.SAVE_MENU)
