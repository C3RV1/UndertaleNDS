import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.debug("Cutscene fall!")
    c.player_control(False)
    c.load_sprite_relative(0, 29-(10//2), "room_sprites/hole", Target(TargetType.PLAYER))
    c.play_sfx("snd_chug.wav")
    c.set_opacity(Target(TargetType.PLAYER), 24)
    c.wait(WaitTypes.FRAMES, 30)
    c.set_animation(Target(TargetType.PLAYER), "spin")
    c.move_in_frames(Target(TargetType.PLAYER), 0, 220, 90)
    c.play_sfx("snd_fall2.wav")
    c.wait(WaitTypes.FRAMES, 90)
    c.set_opacity(Target(TargetType.PLAYER), 31)
    c.set_animation(Target(TargetType.PLAYER), "downIdle")
