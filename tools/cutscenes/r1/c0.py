import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


FLOWEY_ATTACK1_FLAG = 220


def cutscene(c: Cutscene):
    c.manual_camera(True)
    c.player_control(False)
    c.move_in_frames(Target(TargetType.CAMERA), 0, -30, 60)
    c.wait(WaitTypes.FRAMES, 120)
    c.start_bgm("mus_flowey.wav", True)
    c.set_animation(Target(TargetType.PLAYER), "upIdle")
    c.dialogue_centered(10, "speaker/flowey", 128 - 21, (192 - 44) // 4 - 5, "nice1", "nice1_talk",
                        Target(TargetType.SPRITE, 0), "gfx", "talk",
                        type_sound="snd_floweytalk1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.start_battle([EnemyID.FLOWEY], 0, 61, 63, 134, 75)
    c.wait(WaitTypes.EXIT)
    c.debug("Loading battle...")

    c.wait(WaitTypes.ENTER)
    c.debug("In battle!")
    c.wait(WaitTypes.FRAMES, 60)
    c.dialogue_left_align(20, 90, 192 // 4, Target(TargetType.ENEMY, 0), "nice1", "nice1_talk",
                          type_sound="snd_floweytalk1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.start_battle_attacks()
    c.wait(WaitTypes.FRAMES, 120)
    c.wait(WaitTypes.BATTLE_ATTACK)

    c.enemy_command(0, FloweyCommands.PROGRESS_TO_ATTACK)

    c.dialogue_left_align(25, 90, 192 // 4, Target(TargetType.ENEMY, 0), "nice1", "nice1_talk",
                          type_sound="snd_floweytalk1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.set_animation(Target(TargetType.ENEMY, 0), "wink")
    c.wait(WaitTypes.FRAMES, 120)

    # Load pellets (sprites 1, 2, 3, 4, 5)
    c.load_sprite(40, 192 // 2, "battle/attack_pellets")
    c.load_sprite(40, 192 // 2, "battle/attack_pellets")
    c.load_sprite(40, 192 // 2, "battle/attack_pellets")
    c.load_sprite(40, 192 // 2, "battle/attack_pellets")
    c.load_sprite(40, 192 // 2, "battle/attack_pellets")

    def set_pellet_pos():
        c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 30, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 1), 70, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 2), 110, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 3), 150, 140, 120)
        c.set_pos_in_frames(Target(TargetType.SPRITE, 4), 190, 140, 120)
        c.wait(WaitTypes.FRAMES, 160)
    set_pellet_pos()

    c.dialogue_left_align(30, 90, 192 // 4, Target(TargetType.ENEMY, 0), "nice2", "nice2_talk",
                          type_sound="snd_floweytalk1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.wait(WaitTypes.FRAMES, 60)

    def pellet_attack():
        c.move_in_frames(Target(TargetType.SPRITE, 0), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 1), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 2), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 3), 0, 70, 60)
        c.move_in_frames(Target(TargetType.SPRITE, 4), 0, 70, 60)
        c.wait(WaitTypes.FRAMES, 60)
        c.start_battle_attacks()
        c.wait(WaitTypes.BATTLE_ATTACK)

    pellet_attack()
    c.cmp_flag(FLOWEY_ATTACK1_FLAG, "==", 1)
    hit_1 = c.jump_if()

    c.dialogue_left_align(40, 90, 192 // 4, Target(TargetType.ENEMY, 0), "sassy", "sassy_talk",
                          type_sound="snd_floweytalk1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.ENEMY, 0), "nice1")

    set_pellet_pos()
    c.wait(WaitTypes.FRAMES, 60)
    pellet_attack()
    c.cmp_flag(FLOWEY_ATTACK1_FLAG, "==", 1)
    hit_2 = c.jump_if()

    c.set_animation(Target(TargetType.ENEMY, 0), "annoyed")
    c.wait(WaitTypes.FRAMES, 60)
    c.dialogue_left_align(50, 90, 192 // 4, Target(TargetType.ENEMY, 0), "annoyed", "annoyed_talk",
                          type_sound="snd_floweytalk1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.wait(WaitTypes.FRAMES, 40)
    c.dialogue_left_align(60, 90, 192 // 4, Target(TargetType.ENEMY, 0), "nice1", "nice1_talk",
                          frames_per_letter=0)
    c.wait(WaitTypes.DIALOGUE)

    set_pellet_pos()
    c.wait(WaitTypes.FRAMES, 60)
    pellet_attack()
    c.cmp_flag(FLOWEY_ATTACK1_FLAG, "==", 1)
    not_hit = c.jump_if_not()

    c.bind(hit_1)
    c.bind(hit_2)
    c.debug("Player got hit!")

    c.stop_bgm()
    c.clear()
    c.set_animation(target=Target(TargetType.ENEMY, 0), animation="skull_idle")
    c.wait(WaitTypes.FRAMES, 2*60 + 30)

    c.dialogue_left_align(71, 90, 192 // 4, Target(TargetType.ENEMY, 0), "skull_idle", "skull_talk",
                          type_sound="snd_floweytalk2.wav")
    c.wait(WaitTypes.DIALOGUE)

    post_no_hit = c.jump()

    c.bind(not_hit)
    c.stop_bgm()

    c.debug("Player avoided getting hit!")
    c.set_animation(Target(TargetType.ENEMY, 0), "evil")
    c.wait(WaitTypes.FRAMES, 40)
    c.dialogue_left_align(70, 90, 192 // 4, Target(TargetType.ENEMY, 0), "evil", "evil_talk",
                          type_sound="snd_floweytalk2.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.bind(post_no_hit)
    c.debug("Branch merge reached!")

    c.dialogue_left_align(80, 80, 192 // 4, Target(TargetType.ENEMY, 0), "evil", "evil_talk",
                          font="fnt_plainbig.font")
    c.wait(WaitTypes.DIALOGUE)

    # Unload pellets
    c.unload_sprite(-1)
    c.unload_sprite(-1)
    c.unload_sprite(-1)
    c.unload_sprite(-1)
    c.unload_sprite(-1)

    c.set_animation(Target(TargetType.ENEMY, 0), "skull_laugh")
    c.play_sfx("snd_floweylaugh.wav")
    c.enemy_command(0, FloweyCommands.PROGRESS_TO_KILL)
    c.start_battle_attacks()
    c.wait(WaitTypes.BATTLE_ATTACK)

    c.set_animation(Target(TargetType.ENEMY, 0), "skull_idle")
    c.wait(WaitTypes.FRAMES, 40)
    c.play_sfx("snd_heal_c.wav")
    c.max_health()
    c.wait(WaitTypes.FRAMES, 90)

    c.set_animation(Target(TargetType.ENEMY, 0), "annoyed_open_mouth")
    c.wait(WaitTypes.FRAMES, 80)

    c.load_sprite(256 - 60, (192 - 30) // 2, "cutscene/0/spr_torielflame")

    c.set_animation(Target(TargetType.SPRITE, 0), "flashing")
    c.wait(WaitTypes.FRAMES, 60)
    c.set_animation(Target(TargetType.SPRITE, 0), "flying")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 30, (192 - 30) // 2, 60)
    c.wait(WaitTypes.FRAMES, 60)
    c.unload_sprite(-1)

    c.set_animation(Target(TargetType.ENEMY, 0), "hurt")
    c.play_sfx("snd_ehurt1.wav")
    c.move_in_frames(Target(TargetType.ENEMY, 0), -100, 0, 60)
    c.wait(WaitTypes.FRAMES, 60)

    c.wait(WaitTypes.FRAMES, 120)
    c.start_bgm("mus_toriel.wav", True)
    c.load_sprite(256, 192 // 4, "speaker/toriel_face")
    c.load_sprite(256, 192 // 4, "speaker/toriel_bodyonly")
    c.set_animation(Target(TargetType.SPRITE, 0), "worried_side")
    c.move_in_frames(Target(TargetType.SPRITE, 0), -220, 0, 180)
    c.move_in_frames(Target(TargetType.SPRITE, 1), -220, 0, 180)
    c.wait(WaitTypes.FRAMES, 180)
    c.dialogue_left_align(90, 100, 192 // 4, Target(TargetType.SPRITE, 0),
                            "worried_side", "worried_side_talk",
                          type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.wait(WaitTypes.FRAMES, 30)

    c.exit_battle()
    c.wait(WaitTypes.EXIT)
    c.debug("Loading room!")

    # Unload flowey and load toriel world
    c.unload_sprite(0)
    c.load_sprite(149, 198, "room_sprites/toriel")  # Toriel world
    c.set_animation(Target(TargetType.SPRITE, 0), "downIdle")

    c.wait(WaitTypes.ENTER)
    c.debug("Entered room!")

    c.dialogue_centered(100, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4, "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, 0), "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.set_pos_in_frames(Target(TargetType.SPRITE, 0), 149, 67, 180)
    c.move_in_frames(Target(TargetType.CAMERA, 0), 0, 67-198, 180)
    c.wait(WaitTypes.FRAMES, 180)
    c.unload_sprite(0)
    c.wait(WaitTypes.FRAMES, 60 * 2)
    c.manual_camera(False)
    c.player_control(True)
    c.set_flag(FlagOffsets.PROGRESS, 1)
    c.set_collider_enabled(0, False)
    c.debug("Meet flowey cutscene end!")
