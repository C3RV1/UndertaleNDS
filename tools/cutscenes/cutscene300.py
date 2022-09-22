import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.set_collider_enabled(0, False)
    c.player_control(False)
    c.manual_camera(True)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -20, 60)
    c.wait_frames(60)
    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk", type_sound="snd_txttor.wav")
    c.wait_dialogue_end()

    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 80, 0, 60)
    c.wait_frames(25)
    c.set_animation(Target(TargetType.SPRITE, 4), "pressed")
    c.wait_frames(20)
    c.set_animation(Target(TargetType.SPRITE, 5), "pressed")
    c.wait_frames(15)

    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -40, 30)
    c.wait_frames(20)
    c.set_animation(Target(TargetType.SPRITE, 2), "pressed")
    c.wait_frames(10)

    c.set_animation(Target(TargetType.SPRITE, 0), "leftMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), -40, 0, 30)
    c.wait_frames(25)
    c.set_animation(Target(TargetType.SPRITE, 1), "pressed")
    c.wait_frames(5)

    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -15, 20)
    c.wait_frames(20)

    c.set_animation(Target(TargetType.SPRITE, 0), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 15, 0, 10)
    c.wait_frames(10)

    c.set_animation(Target(TargetType.SPRITE, 0), "upIdle")
    c.set_animation(Target(TargetType.SPRITE, 7), "down")

    c.wait_frames(60)

    c.set_animation(Target(TargetType.SPRITE, 0), "leftMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), -70, 0, 60)
    c.wait_frames(60)
    c.set_animation(Target(TargetType.SPRITE, 0), "downMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, 30, 20)
    c.wait_frames(20)

    c.set_animation(Target(TargetType.SPRITE, 0), "downIdle")
    c.wait_frames(20)
    c.start_dialogue(20, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk", type_sound="snd_txttor.wav")
    c.wait_dialogue_end()

    c.wait_frames(30)
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -50, 40)
    c.wait_frames(40)
    c.unload_sprite(0)
    c.unload_texture(0)
    c.wait_frames(30)

    c.move_in_frames(Target(TargetType.CAMERA), 0, 20, 60)

    c.set_flag(0, 3)
