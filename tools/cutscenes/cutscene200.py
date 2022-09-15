import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 128, 200, 90)
    c.set_collider_enabled(0, False)
    c.wait_frames(90)
    c.set_animation(Target(TargetType.SPRITE, 0), "downIdle")
