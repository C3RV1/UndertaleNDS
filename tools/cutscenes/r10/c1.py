import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.load_sprite_relative(0, 0, 0, Target(TargetType.PLAYER))
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.wait(WaitTypes.FRAMES, 30)
    c.set_animation(Target(TargetType.PLAYER), "spin")
    c.move_in_frames(Target(TargetType.PLAYER), 0, 220, 120)
    c.wait(WaitTypes.FRAMES, 120)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
