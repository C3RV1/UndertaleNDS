import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.SPRITE, 2), "down")
    c.set_interact_action(Target(TargetType.SPRITE, 2), "none")
    c.set_collider_enabled(2, False)

    # TODO: Screen shake
    c.set_animation(Target(TargetType.SPRITE, 5), "down")
    c.set_animation(Target(TargetType.SPRITE, 6), "down")

    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 7),
                     "leftIdle", "leftTalk", type_sound="snd_txttor.wav")
    c.wait_dialogue_end()
    c.player_control(True)

    c.set_animation(Target(TargetType.SPRITE, 7), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 7), 700 - 640, 0, 60)
    c.wait_frames(60)
    c.unload_sprite(7)
    c.unload_texture(4)

    c.set_flag(0, 4)
