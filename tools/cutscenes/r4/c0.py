import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.player_control(False)
    c.start_dialogue(10, "", 0, 0, "", "", Target(TargetType.NULL), "", "",
                     type_sound="SND_TXT1.wav")
    c.wait_dialogue_end()
