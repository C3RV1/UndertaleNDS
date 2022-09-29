import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, WaitTypes
else:
    from CutsceneTypes import Cutscene, Target, TargetType, WaitTypes


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 233, 92, 80)
    c.set_collider_enabled(1, False)
    c.wait(WaitTypes.FRAMES, 80)
    c.set_animation(Target(TargetType.SPRITE, 0), "downIdle")
