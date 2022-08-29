from CutsceneTypes import Cutscene, Target, TargetType


def cutscene(c: Cutscene):
    c.start_dialogue(0,
                     "spr/speaker/flowey.cspr", 0, 0, "nice1", "nice1_talk",
                     Target(TargetType.SPRITE, 0), "idle", "talk")
    c.wait_dialogue_end()

    c.start_battle()
    c.load_sprite(0, 0, "spr/speaker/flowey.cspr")  # flowey
    c.wait_frames(30)
    c.load_sprite(0, 0, "spr/dialogue/spr_blconwdshrt.cspr")  # dialogue box
    c.wait_load()
    c.start_battle_dialogue(0, 0, 1, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk", 300)
    c.wait_dialogue_end()

    c.hide(Target(TargetType.SPRITE, 1))
    c.set_animation(Target(TargetType.SPRITE, 0), "wink")
    c.wait_frames(180)
    c.show(Target(TargetType.SPRITE, 1))

    c.load_sprite(0, 0, "")
    c.load_sprite(0, 0, "")
    c.load_sprite(0, 0, "")
    c.load_sprite(0, 0, "")
    c.load_sprite(0, 0, "")
    c.move_in_frames(Target(TargetType.SPRITE, 2), 0, 0, 180)
    c.move_in_frames(Target(TargetType.SPRITE, 3), 0, 0, 180)
    c.move_in_frames(Target(TargetType.SPRITE, 4), 0, 0, 180)
    c.move_in_frames(Target(TargetType.SPRITE, 5), 0, 0, 180)
    c.move_in_frames(Target(TargetType.SPRITE, 6), 0, 0, 180)

    c.start_battle_dialogue(0, 0, 2, Target(TargetType.SPRITE, 0), "nice2", "nice2_talk", 300)
    c.wait_dialogue_end()

    c.start_battle_dialogue(0, 0, 3, Target(TargetType.SPRITE, 0), "nice1", "nice1_talk", 180)

    def do_attack():
        c.move_in_frames(Target(TargetType.SPRITE, 2), 0, 0, 120)
        c.move_in_frames(Target(TargetType.SPRITE, 3), 0, 0, 120)
        c.move_in_frames(Target(TargetType.SPRITE, 4), 0, 0, 120)
        c.move_in_frames(Target(TargetType.SPRITE, 5), 0, 0, 120)
        c.move_in_frames(Target(TargetType.SPRITE, 6), 0, 0, 120)

        c.wait_frames(120)
        c.battle_attack(0)
        c.wait_battle_attack()

    do_attack()
    c.check_hit()
    jump_hit = c.jump_if()

    c.start_battle_dialogue(0, 0, 4, Target(TargetType.SPRITE, 0), "sassy", "sassy_talk", 300)
    c.wait_dialogue_end()

    def set_pellets():
        c.set_x_y(Target(TargetType.SPRITE, 2), 0, 0)
        c.set_x_y(Target(TargetType.SPRITE, 3), 0, 0)
        c.set_x_y(Target(TargetType.SPRITE, 4), 0, 0)
        c.set_x_y(Target(TargetType.SPRITE, 5), 0, 0)
        c.set_x_y(Target(TargetType.SPRITE, 6), 0, 0)

    set_pellets()
    do_attack()

    c.check_hit()
    jump_hit2 = c.jump_if()

    c.start_battle_dialogue(0, 0, 5, Target(TargetType.SPRITE, 0), "annoyed", "annoyed_talk", 300)
    c.wait_frames(180)

    set_pellets()
    do_attack()

    c.check_hit()
    jump_hit3 = c.jump_if()

    c.start_battle_dialogue(0, 0, 6, Target(TargetType.SPRITE, 0), "evil", "evil_talk", 300)
    c.wait_dialogue_end()
    die_jump = c.jump()

    c.bound_jump(jump_hit)
    c.bound_jump(jump_hit2)
    c.bound_jump(jump_hit3)

    c.start_battle_dialogue(0, 0, 7, Target(TargetType.SPRITE, 0), "grin", "grin_talk", 300)

    c.bound_jump(die_jump)
    c.start_battle_dialogue(0, 0, 8, Target(TargetType.SPRITE, 0), "evil", "evil", 120)
    c.battle_attack(1)
    c.wait_frames(120)
    c.hide(Target(TargetType.SPRITE, 1))
    c.set_animation(Target(TargetType.SPRITE, 0), "skull_laugh")
    c.wait_battle_attack()

    c.wait_frames(120)
    c.set_animation(Target(TargetType.SPRITE, 0), "annoyed")
    c.wait_frames(120)
    c.load_sprite(0, 0, "")  # toriel ball
    c.wait_frames(120)
    c.move_in_frames(Target(TargetType.SPRITE, 7), 0, 0, 0)
    c.wait_frames(120)
    c.set_animation(Target(TargetType.SPRITE, 0), "hurt")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, 0, 0)
    c.wait_frames(120)
    c.load_sprite(0, 0, "")  # toriel
    c.set_animation(Target(TargetType.SPRITE, 8), "")
    c.move_in_frames(Target(TargetType.SPRITE, 8), 0, 0, 0)
    c.wait_frames(120)
    c.set_x_y(Target(TargetType.SPRITE, 1), 0, 0)
    c.show(Target(TargetType.SPRITE, 1))
    c.start_battle_dialogue(0, 0, 9, Target(TargetType.SPRITE, 8), "worried", "worried_talk", 0)
    c.wait_dialogue_end()

    c.exit_battle()

    c.load_sprite(0, 0, "")  # Toriel world
    c.wait_load()
    c.start_dialogue(10, "", 0, 0, "idle", "talk", Target(TargetType.SPRITE), "idle", "talk")
    c.wait_dialogue_end()
    c.move_in_frames(Target(TargetType.SPRITE, 2), 0, 0, 0)
    c.player_control(True)
    c.wait_frames(0)
