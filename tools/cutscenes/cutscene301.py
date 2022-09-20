import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.player_control(False)
    c.start_dialogue(10, "", 0, 0, "", "", Target(TargetType.NULL), "", "",
                     "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.player_control(True)
