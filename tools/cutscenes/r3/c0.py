import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


LEVER_ID = 5
DOOR_ID = 6


def cutscene(c: Cutscene):
    c.set_collider_enabled(1, False)
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.manual_camera(True)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -20, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
                     "downIdle", "downTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 80, 0, 60)
    c.wait(WaitTypes.FRAMES, 25)
    c.set_animation(Target(TargetType.SPRITE, 1), "pressed")
    c.wait(WaitTypes.FRAMES, 20)
    c.set_animation(Target(TargetType.SPRITE, 2), "pressed")
    c.wait(WaitTypes.FRAMES, 15)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 0, -40, 30)
    c.wait(WaitTypes.FRAMES, 20)
    c.set_animation(Target(TargetType.SPRITE, 3), "pressed")
    c.wait(WaitTypes.FRAMES, 10)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "leftMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), -40, 0, 30)
    c.wait(WaitTypes.FRAMES, 25)
    c.set_animation(Target(TargetType.SPRITE, 4), "pressed")
    c.wait(WaitTypes.FRAMES, 5)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 0, -15, 20)
    c.wait(WaitTypes.FRAMES, 20)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 15, 0, 10)
    c.wait(WaitTypes.FRAMES, 10)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upIdle")

    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.SPRITE, LEVER_ID), "down")
    c.unload_sprite(DOOR_ID)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "leftMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), -70, 0, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 0, 30, 20)
    c.wait(WaitTypes.FRAMES, 20)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "downIdle")
    c.wait(WaitTypes.FRAMES, 20)
    c.dialogue_centered(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
                     "downIdle", "downTalk", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.wait(WaitTypes.FRAMES, 30)
    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 0, -50, 40)
    c.wait(WaitTypes.FRAMES, 40)
    c.unload_sprite(SpriteIDs.TORIEL_ROOM)
    c.wait(WaitTypes.FRAMES, 30)

    c.move_in_frames(Target(TargetType.CAMERA), 0, 20, 60)

    c.set_flag(FlagOffsets.PROGRESS, 3)
