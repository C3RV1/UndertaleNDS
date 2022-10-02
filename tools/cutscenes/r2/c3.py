import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 127, 43, 20)
    c.set_collider_enabled(3, False)
    c.wait(WaitTypes.FRAMES, 20)
    c.unload_sprite(0)
    c.unload_texture(0)
    c.set_flag(FlagOffsets.PROGRESS, 2)
