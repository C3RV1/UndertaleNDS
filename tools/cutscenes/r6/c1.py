import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(1, False)
    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 455 - 155, 0,
                     frames_from_dst(455 - 155, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(455 - 155, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 0), "downMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, 127 - 86,
                     frames_from_dst(127 - 86, PLAYER_SPEED))
    c.wait(WaitTypes.FRAMES, frames_from_dst(127 - 86, PLAYER_SPEED))
    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 635 - 459, 0,
                     frames_from_dst(635 - 459, PLAYER_SPEED))
    # Do not wait, exit cutscene right away to allow new cutscene
    # c.wait(WaitTypes.FRAMES, frames_from_dst(635 - 459, 70))
