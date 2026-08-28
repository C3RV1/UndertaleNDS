import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.set_flag(FlagOffsets.PROGRESS, 2)
    c.set_pos_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 127, 43, 20)
    c.set_collider_enabled(4, False)
    c.wait(WaitTypes.FRAMES, 20)
    c.unload_sprite(SpriteIDs.TORIEL_ROOM)
