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
    c.start_dialogue(10, "speaker/flowey.cspr", 128 - 21, (192 - 44) // 4 - 5, "nice1", "nice1_talk",
                     Target(TargetType.SPRITE, 0), "idle", "talk", "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()

    c.start_battle([], 0, 61, 63, 134, 75)
    c.wait_exit()
    c.debug("Loading battle...")
    c.load_texture("speaker/flowey.cspr")
    c.load_texture("cutscene/0/spr_torielflame.cspr")
    c.load_texture("battle/attack_pellets.cspr")
    c.load_sprite(30, (192 - 44) // 2, 0)  # Load flowey (spr 0)

    c.wait_enter()
    c.debug("In battle!")
    c.wait_frames(60)
    c.start_dialogue_battle(20, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.battle_attack(1)
    c.wait_frames(120)
    c.wait_battle_attack()
    c.start_dialogue_battle(25, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.set_animation(Target(TargetType.SPRITE, 0), "wink")
    c.wait_frames(120)

    # Load pellets (sprites 1, 2, 3, 4, 5)
    c.load_sprite(40, 192 // 2, 2)
    c.load_sprite(40, 192 // 2, 2)
    c.load_sprite(40, 192 // 2, 2)
    c.load_sprite(40, 192 // 2, 2)
    c.load_sprite(40, 192 // 2, 2)
    c.set_animation(Target(TargetType.SPRITE, 1), "gfx")
    c.set_animation(Target(TargetType.SPRITE, 2), "gfx")
    c.set_animation(Target(TargetType.SPRITE, 3), "gfx")
    c.set_animation(Target(TargetType.SPRITE, 4), "gfx")
    c.set_animation(Target(TargetType.SPRITE, 5), "gfx")
    def set_pellet_pos():
        c.set_pos_in_frames(Target(TargetType.SPRITE, 1), 30, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 2), 70, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 3), 110, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 4), 150, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 5), 190, 140, 120)
        c.wait_frames(160)
    set_pellet_pos()

    c.start_dialogue_battle(30, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice2", "nice2_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.wait_frames(60)

    def pellet_attack():
        c.move_in_frames(Target(TargetType.SPRITE, 1), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 2), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 3), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 4), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 5), 0, 70, 60)
        c.wait_frames(60)
        c.battle_attack(2)
        c.wait_battle_attack()

    pellet_attack()
    c.check_hit()
    hit_1 = c.jump_if()

    c.start_dialogue_battle(40, 90, 192 // 4, Target(TargetType.SPRITE, 0), "sassy", "sassy_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.wait_frames(60)
    c.set_animation(Target(TargetType.SPRITE, 0), "nice1")

    set_pellet_pos()
    c.wait_frames(60)
    pellet_attack()
    c.check_hit()
    hit_2 = c.jump_if()

    c.set_animation(Target(TargetType.SPRITE, 0), "annoyed")
    c.wait_frames(60)
    c.start_dialogue_battle(50, 90, 192 // 4, Target(TargetType.SPRITE, 0), "annoyed", "annoyed_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.wait_frames(40)
    c.start_dialogue_battle(60, 90, 192 // 4, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk",
                            "fnt_maintext.font.cfnt", frames_per_letter=0)
    c.wait_dialogue_end()

    set_pellet_pos()
    c.wait_frames(60)
    pellet_attack()
    c.check_hit()
    not_hit = c.jump_if_not()

    c.bind(hit_1)
    c.bind(hit_2)

    c.start_dialogue_battle(71, 90, 192 // 4, Target(TargetType.SPRITE, 0), "skull_idle", "skull_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()

    post_no_hit = c.jump()

    c.bind(not_hit)
    c.stop_bgm()

    c.set_animation(Target(TargetType.SPRITE, 0), "evil")
    c.wait_frames(40)
    c.start_dialogue_battle(70, 90, 192 // 4, Target(TargetType.SPRITE, 0), "evil", "evil_talk",
                            "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()

    c.bind(post_no_hit)

    c.start_dialogue_battle(7, 80, 192 // 4, Target(TargetType.SPRITE, 0), "evil", "evil_talk",
                            "fnt_plainbig.font.cfnt")
    c.wait_dialogue_end()

    # Unload pellets
    c.unload_sprite(5)
    c.unload_sprite(4)
    c.unload_sprite(3)
    c.unload_sprite(2)
    c.unload_sprite(1)
    c.unload_texture(2)

    c.set_animation(Target(TargetType.SPRITE, 0), "skull_laugh")
    c.wait_frames(240)

    c.set_animation(Target(TargetType.SPRITE, 0), "skull_idle")
    c.wait_frames(80)

    c.load_sprite(256 - 60, (192 - 30) // 2, 1)
    c.set_animation(Target(TargetType.SPRITE, 1), "flashing")
    c.wait_frames(40)
    c.set_animation(Target(TargetType.SPRITE, 1), "flying")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 1), 30, (192 - 30) // 2, 120)
    c.wait_frames(120)
    c.unload_sprite(1)
    c.unload_texture(1)
    c.set_animation(Target(TargetType.SPRITE, 0), "hurt")
    c.move_in_frames(Target(TargetType.SPRITE, 0), -100, 0, 60)
    c.wait_frames(60)
    c.unload_sprite(0)
    c.unload_texture(1)

    c.wait_frames(300)

    c.exit_battle()
    c.wait_exit()
    c.debug("Loading room!")
    c.wait_enter()
    c.debug("Entered room again")
    c.wait_frames(180)

    c.move_in_frames(Target(TargetType.SPRITE, 0), -200, 0, 60)
    c.wait_frames(60)
    c.unload_sprite(0)
    c.unload_texture(0)

    c.load_texture("room_sprites/toriel.cspr")
    c.load_sprite(149, 198, 0)  # Toriel world
    c.set_animation(Target(TargetType.SPRITE, 0), "downIdle")
    c.start_dialogue(10, "speaker/toriel.cspr", (256 - 50) // 2, (192 - 39) // 4, "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 0), "downIdle", "downTalk",
                     "fnt_maintext.font.cfnt")
    c.wait_dialogue_end()
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 149, 67, 180)
    c.move_in_frames(Target(TargetType.CAMERA, 0), 0, 67-198, 180)
    c.wait_frames(180)
    c.unload_sprite(0)
    c.unload_texture(0)
    c.wait_frames(60 * 4)
    c.manual_camera(False)
    c.player_control(True)
    c.set_flag(0, 1)
    c.set_collider_enabled(0, False)
    c.debug("Cutscene 2 end!")
