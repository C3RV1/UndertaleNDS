import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.debug("Debug cutscene 1!")
    c.wait_frames(180)
    c.set_animation(Target(TargetType.SPRITE, 0), "talk")
    c.set_animation(Target(TargetType.PLAYER), "downMove")
    c.wait_frames(180)
    c.debug("Player control & manual cam 5 seconds!")
    c.player_control(True)
    c.manual_camera(True)
    c.wait_frames(300)
    c.debug("End camera manual!")
    c.manual_camera(False)
    c.wait_frames(180)
    c.move_in_frames(Target(TargetType.SPRITE, 0), 100, 100, 180)
    c.scale_in_frames(Target(TargetType.SPRITE, 0), 2, 0.5, 180)
    c.wait_load()
    c.debug("End debug cutscene!")
