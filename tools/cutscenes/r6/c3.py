import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(3, False)
    c.set_action(Target(TargetType.SPRITE, 0), "cutscene", 4)
    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 638 - 612, 0,
                     frames_from_dst(638 - 612, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(638 - 612, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, 103 - 127,
                     frames_from_dst(103 - 127, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(103 - 127, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 764 - 638, 0,
                     frames_from_dst(764 - 638, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(764 - 638, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 0), "rightIdle")
