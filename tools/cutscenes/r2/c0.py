import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 128, 200, 60)
    c.set_collider_enabled(1, False)
    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downIdle")
