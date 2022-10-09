import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_collider_enabled(2, False)
    c.load_texture("room_sprites/spr_exc")
    c.load_sprite_relative(5, -13, 2, Target(TargetType.PLAYER))
    c.wait(WaitTypes.FRAMES, 60)

    c.unload_sprite(-1)
    c.unload_texture(-1)

    c.start_battle([], 0, 61, 63, 134, 75)
    c.wait(WaitTypes.EXIT)

    c.debug("Battle loop!")
