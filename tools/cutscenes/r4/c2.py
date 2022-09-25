import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 7),
                     "downIdle", "downTalk",
                     type_sound="snd_txttor.wav")
    c.wait_dialogue_end()
    c.player_control(True)

    c.set_animation(Target(TargetType.SPRITE, 7), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, 7), 425-98, 0, 180)
    c.wait_frames(180)

    c.set_animation(Target(TargetType.SPRITE, 7), "leftIdle")
    c.set_interact_action(Target(TargetType.SPRITE, 7), "cutscene",
                          cutscene_id=10)

    c.set_collider_enabled(0, False)
