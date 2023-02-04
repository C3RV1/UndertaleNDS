import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.manual_camera(True)
    c.set_animation(Target(TargetType.PLAYER), "downIdle")
    c.move(Target(TargetType.PLAYER), 0, -200)
    c.wait(WaitTypes.FRAMES, 10)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -200, 60)
    c.wait(WaitTypes.FRAMES, 60)
