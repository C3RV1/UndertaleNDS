import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.manual_camera(True)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -30, 60)
    c.wait_frames(120)
    c.start_bgm("nitro:/z_audio/mus_flowey.wav", True)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.start_dialogue(0, "nitro:/spr/speaker/flowey.cspr", 128 - 21, (192 - 44) // 4 - 5, "nice1", "nice1_talk",
                     Target(TargetType.SPRITE, 0), "idle", "talk", "nitro:/fnt/fnt_main.font.cfnt")
    c.wait_dialogue_end()
    c.move_in_frames(Target(TargetType.SPRITE, 0), -200, 0, 60)
    c.wait_frames(60)
    c.set_shown(Target(TargetType.SPRITE, 0), False)

    c.load_sprite(149, 198, "nitro:/spr/room_sprites/toriel.cspr")  # Toriel world
    c.set_animation(Target(TargetType.SPRITE, 1), "downIdle")
    c.start_dialogue(10, "nitro:/spr/speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4, "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 1), "downIdle", "downTalk",
                     "nitro:/fnt/fnt_main.font.cfnt")
    c.wait_dialogue_end()
    c.set_animation(Target(TargetType.SPRITE, 1), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 1), 149, 67, 180)
    c.move_in_frames(Target(TargetType.CAMERA, 1), 0, 67-198, 180)
    c.wait_frames(180)
    c.unload_sprite(1)
    c.wait_frames(60 * 4)
    c.manual_camera(False)
    c.unload_sprite(1)
    c.player_control(True)
    c.set_flag(0, 1)
    c.set_collider_enabled(0, False)
    c.debug("Cutscene 2 end!")
