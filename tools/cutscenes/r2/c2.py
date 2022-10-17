import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, WaitTypes
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), -20, -25, 20)
    c.wait(WaitTypes.FRAMES, 20)
    c.set_animation(Target(TargetType.SPRITE, 0), 'leftMove')
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 127, 62, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.SPRITE, 0), "downIdle")
    c.set_collider_enabled(2, False)
