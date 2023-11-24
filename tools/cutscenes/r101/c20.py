import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    # == Sans Between Trees ==
    c.debug("Sans Between The Trees")
    c.set_collider_enabled(1, False)
    c.load_sprite(1260, 165, 3)
    c.set_animation(Target(TargetType.SPRITE, -1), "rightMoveDark")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 20, 0, 20)
    c.wait(WaitTypes.FRAMES, 20)
    c.unload_sprite(-1)