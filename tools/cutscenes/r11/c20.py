import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *

def cutscene(c: Cutscene):
    c.debug("Goal cutscene!")
    c.player_control(True)
    c.set_animation(target=Target(target_type=TargetType.SPRITE, target_id=2), animation="down")
    c.set_animation(target=Target(target_type=TargetType.SPRITE, target_id=3), animation="down")
    c.set_animation(target=Target(target_type=TargetType.SPRITE, target_id=4), animation="down")
    c.set_animation(target=Target(target_type=TargetType.SPRITE, target_id=5), animation="down")
    c.set_animation(target=Target(target_type=TargetType.SPRITE, target_id=6), animation="down")
    c.set_animation(target=Target(target_type=TargetType.SPRITE, target_id=7), animation="down")
    c.set_collider_enabled(enabled=False, collider_id=0)
    c.play_sfx(path="snd_screenshake.wav")

    c.set_flag(flag_id=FlagOffsets.PROGRESS, flag_value=10)

