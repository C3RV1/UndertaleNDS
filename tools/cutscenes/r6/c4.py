import typing

if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (
        Cutscene,
        Target,
        TargetType,
        WaitTypes,
        frames_from_dst,
        SpriteIDs,
        FlagOffsets
    )
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.set_collider_enabled(5, False)
    c.player_control(False)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "leftIdle")
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")

    c.dialogue_centered(
        10,
        "speaker/toriel",
        (256 - 50) // 2,
        (192 - 39) // 4,
        "worriedLeftIdle",
        "worriedLeftTalk",
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        "leftIdle",
        "leftTalk",
        type_sound="snd_txttor.wav",
    )
    c.wait(WaitTypes.DIALOGUE)

    c.load_sprite(
        SpriteIDs.TORIEL_HANDHOLD_ROOM, 743, 103, "room_sprites/toriel_handhold"
    )
    c.set_shown(Target(TargetType.PLAYER), False)
    c.set_shown(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), False)
    c.set_pos(Target(TargetType.PLAYER), 742, 128)

    speed = 40

    c.set_animation(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "rightMove"
    )
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        80,
        0,
        frames_from_dst(80, speed),
    )
    c.set_pos_in_frames(Target(TargetType.PLAYER), 840, 128, frames_from_dst(80, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(80, speed))

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "upMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        0,
        -40,
        frames_from_dst(40, speed),
    )
    c.set_pos_in_frames(Target(TargetType.PLAYER), 840, 88, frames_from_dst(40, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(40, speed))

    c.set_animation(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "rightMove"
    )
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        60,
        0,
        frames_from_dst(60, speed),
    )
    c.set_pos_in_frames(Target(TargetType.PLAYER), 900, 88, frames_from_dst(60, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(60, speed))

    c.set_animation(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "downMove"
    )
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        0,
        40,
        frames_from_dst(40, speed),
    )
    c.set_pos_in_frames(Target(TargetType.PLAYER), 900, 128, frames_from_dst(40, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(40, speed))

    c.set_animation(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "rightMove"
    )
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        100,
        0,
        frames_from_dst(100, speed),
    )
    c.set_pos_in_frames(
        Target(TargetType.PLAYER, 0), 1000, 128, frames_from_dst(100, speed)
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(100, speed))

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "upMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        0,
        -35,
        frames_from_dst(35, speed),
    )
    c.set_pos_in_frames(
        Target(TargetType.PLAYER), 1000, 88, frames_from_dst(35, speed)
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(35, speed))

    c.set_animation(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM), "rightMove"
    )
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_HANDHOLD_ROOM),
        90,
        0,
        frames_from_dst(90, speed),
    )
    c.set_pos_in_frames(Target(TargetType.PLAYER), 1074, 91, frames_from_dst(90, speed))
    c.wait(WaitTypes.FRAMES, frames_from_dst(90, speed))

    c.set_shown(Target(TargetType.PLAYER), True)
    c.set_shown(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), True)
    c.unload_sprite(SpriteIDs.TORIEL_HANDHOLD_ROOM)
    c.set_pos(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 1095, 68)
    c.set_pos(Target(TargetType.PLAYER), 1074, 91)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "leftIdle")
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")

    c.dialogue_centered(
        20,
        "speaker/toriel",
        (256 - 50) // 2,
        (192 - 39) // 4,
        "worriedLeftIdle",
        "worriedLeftTalk",
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
        "leftIdle",
        "leftTalk",
        type_sound="snd_txttor.wav",
    )
    c.wait(WaitTypes.DIALOGUE)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(
        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 50, 0, frames_from_dst(50, 90)
    )
    c.wait(WaitTypes.FRAMES, frames_from_dst(50, 90))
    c.unload_sprite(SpriteIDs.TORIEL_ROOM)
    c.set_flag(FlagOffsets.PROGRESS, 6)
