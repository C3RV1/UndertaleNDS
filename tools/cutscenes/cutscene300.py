import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.set_collider_enabled(0, False)
    c.player_control(False)
    c.manual_camera(True)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -20, 60)
    c.wait_frames(60)
    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk", "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.move_in_frames(Target(TargetType.CAMERA), 0, 20, 60)
    c.wait_frames(60)
