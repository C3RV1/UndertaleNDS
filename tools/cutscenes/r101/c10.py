import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.debug("Stick break!")
    c.set_animation(Target(TargetType.SPRITE, 0), "broken")
    c.set_collider_enabled(0, False)
    c.wait(WaitTypes.FRAMES, 5)
    c.play_sfx("mus_sticksnap.wav", 0)