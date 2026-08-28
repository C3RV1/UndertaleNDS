import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 233, 92, 80)
    c.set_collider_enabled(2, False)
    c.wait(WaitTypes.FRAMES, 80)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downIdle")
