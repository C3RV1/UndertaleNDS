import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), -20, -25, 20)
    c.wait(WaitTypes.FRAMES, 20)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 'leftMove')
    c.set_pos_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 127, 62, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downIdle")
    c.set_collider_enabled(3, False)
