import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import (Cutscene, Target, TargetType, SpriteIDs,
        WaitTypes)
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.dialogue_centered(10, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM),
                     "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.player_control(True)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "rightMove")
    c.move_in_frames(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), 425-98, 0, 180)
    c.wait(WaitTypes.FRAMES, 180)

    c.set_animation(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "leftIdle")
    c.set_action(Target(TargetType.SPRITE, SpriteIDs.TORIEL_ROOM), "cutscene",
                 cutscene_id=10)

    c.set_collider_enabled(10, False)
