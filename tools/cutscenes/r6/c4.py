import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(4, False)
    c.player_control(False)
    c.set_animation(Target(TargetType.SPRITE, 0), "leftIdle")
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")

    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "worriedLeftIdle", "worriedLeftTalk",
                        Target(TargetType.SPRITE, 0),
                     "leftIdle", "leftTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.load_texture("room_sprites/toriel_handhold")
    c.load_sprite(743, 103, -1)
    c.set_shown(Target(TargetType.PLAYER), False)
    c.set_shown(Target(TargetType.SPRITE, 0), False)
    c.set_pos(Target(TargetType.PLAYER), 742, 128)

    speed = 40

    c.set_animation(Target(TargetType.SPRITE, -1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 80, 0,
                     frames_from_dst(80, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER), 840, 128,
                        frames_from_dst(80, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(80, speed))

    c.set_animation(Target(TargetType.SPRITE, -1), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 0, -40,
                     frames_from_dst(40, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER), 840, 88,
                        frames_from_dst(40, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(40, speed))

    c.set_animation(Target(TargetType.SPRITE, -1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 60, 0,
                     frames_from_dst(60, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER), 900, 88,
                        frames_from_dst(60, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(60, speed))

    c.set_animation(Target(TargetType.SPRITE, -1), "downMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 0, 40,
                     frames_from_dst(40, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER), 900, 128,
                        frames_from_dst(40, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(40, speed))

    c.set_animation(Target(TargetType.SPRITE, -1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 100, 0,
                     frames_from_dst(100, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER, 0), 1000, 128,
                        frames_from_dst(100, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(100, speed))

    c.set_animation(Target(TargetType.SPRITE, -1), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 0, -35,
                     frames_from_dst(35, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER, 0), 1000, 88,
                        frames_from_dst(35, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(35, speed))

    c.set_animation(Target(TargetType.SPRITE, -1), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, -1), 90, 0,
                     frames_from_dst(90, speed))
    c.set_pos_in_frames(Target(TargetType.PLAYER), 1074, 91,
                        frames_from_dst(90, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(90, speed))

    c.set_shown(Target(TargetType.PLAYER), True)
    c.set_shown(Target(TargetType.SPRITE, 0), True)
    c.unload_sprite(-1)
    c.unload_texture(-1)
    c.set_pos(Target(TargetType.SPRITE, 0), 1095, 68)
    c.set_pos(Target(TargetType.PLAYER), 1074, 91)
    c.set_animation(Target(TargetType.SPRITE, 0), "leftIdle")
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")

    c.dialogue_centered(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "worriedLeftIdle", "worriedLeftTalk",
                        Target(TargetType.SPRITE, 0),
                     "leftIdle", "leftTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 50, 0,
                     frames_from_dst(50, 90))
    c.wait(WaitTypes.FRAMES, frames_from_dst(50, 90))
    c.unload_sprite(0)
    c.unload_texture(1)
    c.set_flag(FlagOffsets.PROGRESS, 6)
