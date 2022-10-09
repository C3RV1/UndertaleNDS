import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(1, False)
    c.set_animation(Target(TargetType.SPRITE, 0), "moveRight")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 455 - 155, 0,
                     ((455 - 155) * 60) // 90)
    c.wait(WaitTypes.FRAMES, ((455 - 155) * 60) // 90)
    c.set_animation(Target(TargetType.SPRITE, 0), "moveDown")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, 127 - 86,
                     ((127 - 86) * 60) // 90)
    c.wait(WaitTypes.FRAMES, ((127 - 86) * 60) // 90)
    c.set_animation(Target(TargetType.SPRITE, 0), "moveRight")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 635 - 459, 0,
                     ((635 - 459) * 60) // 90)
    # Do not wait, exit cutscene right away to allow new cutscene
    # c.wait(WaitTypes.FRAMES, ((635 - 459) * 60) // 90)
