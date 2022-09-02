import typing
if typing.TYPE_CHECKING:
    from ..CutsceneTypes import Cutscene, Target, TargetType
else:
    from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.manual_camera(True)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -30, 60)
    c.wait_frames(120)
    c.start_bgm("mus_flowey.wav", True)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.start_dialogue(0, "speaker/flowey.cspr", 128 - 21, (192 - 44) // 4 - 5, "nice1", "nice1_talk",
                     Target(TargetType.SPRITE, 0), "idle", "talk", "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()

    c.start_battle([])
    c.load_sprite(30, (192 - 44) // 2, "speaker/flowey.cspr")
    c.wait_enter()
    c.debug("In battle!")
    c.wait_frames(60)
    c.start_dialogue_battle(1, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.set_animation(Target(TargetType.SPRITE, 0), "wink")
    c.wait_frames(180)
    c.start_dialogue_battle(2, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice2", "nice2_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()

    c.check_hit()

    c.start_dialogue_battle(3, 90, 192 // 4, Target(TargetType.SPRITE, 0), "sassy", "sassy_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.set_animation(Target(TargetType.SPRITE, 0), "nice1")

    c.check_hit()

    c.set_animation(Target(TargetType.SPRITE, 0), "annoyed")
    c.wait_frames(60)
    c.start_dialogue_battle(4, 90, 192 // 4, Target(TargetType.SPRITE, 0), "annoyed", "annoyed_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.wait_frames(40)
    c.start_dialogue_battle(5, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk",
                            "fnt_maintext.font.cfnt", frames_per_letter=0)
    c.wait_dialogue_end()

    c.set_animation(Target(TargetType.SPRITE, 0), "evil")
    c.wait_frames(40)
    c.start_dialogue_battle(6, 90, 192 // 4, Target(TargetType.SPRITE, 0), "evil", "evil_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()

    c.start_dialogue_battle(7, 90, 192 // 4, Target(TargetType.SPRITE, 0), "evil", "evil_talk",
                            "fnt_plainbig.font.cfnt")
    c.wait_dialogue_end()

    c.set_animation(Target(TargetType.SPRITE, 0), "skull_laugh")
    c.wait_frames(240)

    c.set_animation(Target(TargetType.SPRITE, 0), "skull_idle")
    c.wait_frames(240)

    c.wait_frames(300)

    c.exit_battle()
    c.wait_exit()
    c.debug("Loading room!")
    c.wait_enter()
    c.debug("Entered room again")
    c.wait_frames(180)

    c.move_in_frames(Target(TargetType.SPRITE, 0), -200, 0, 60)
    c.wait_frames(60)
    c.set_shown(Target(TargetType.SPRITE, 0), False)

    c.load_sprite(149, 198, "room_sprites/toriel.cspr")  # Toriel world
    c.set_animation(Target(TargetType.SPRITE, 1), "downIdle")
    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4, "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 1), "downIdle", "downTalk",
                     "fnt_maintext.font.cfnt")
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
