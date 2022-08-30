import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.debug("Debug cutscene 1!")
    c.wait_frames(180)
    c.debug("Player control & manual cam 5 seconds!")
    c.player_control(True)
    c.manual_camera(True)
    c.wait_frames(300)
    c.debug("End player control!")
    c.player_control(False)
    c.wait_frames(180)
    c.debug("End debug cutscene!")
