import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *

def cutscene(c: Cutscene):
    c.player_control(True)
    c.debug("Goal cutscene!")
