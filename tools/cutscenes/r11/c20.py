import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, FlagOffsets
else:
    from CutsceneTypes import *

SPIKE_IDS = [1, 2, 3, 4, 5, 6]

def cutscene(c: Cutscene):
    c.debug("Goal cutscene!")
    c.player_control(True)
    for i in SPIKE_IDS:
        c.set_animation(Target(TargetType.SPRITE, i), "down")
    c.set_collider_enabled(3, False)
    c.play_sfx("snd_screenshake.wav")

    c.set_flag(FlagOffsets.PROGRESS, 11)

