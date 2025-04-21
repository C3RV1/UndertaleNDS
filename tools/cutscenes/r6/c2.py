import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_collider_enabled(2, False)
    c.load_sprite_relative(5, -13, "room_sprites/spr_exc", Target(TargetType.PLAYER))
    c.play_sfx("snd_b.wav")
    c.wait(WaitTypes.FRAMES, 60)

    c.unload_sprite(-1)

    c.start_battle([2,], 0, 61, 63, 134, 75)
    c.wait(WaitTypes.EXIT)

    c.start_bgm("mus_battle1.wav", True)

    c.wait(WaitTypes.ENTER)

    c.debug("Battle loop!")

    c.battle_action(3)
    c.wait(WaitTypes.BATTLE_ACTION)

    c.cmp_enemy_hp(0, "<", 20)
    fight_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.ACT)
    check_jump = c.jump_if()

    # If we chose Compliment
    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.ACT + 1)
    compliment_jump = c.jump_if()

    # TODO: Threaten.

    toriel_scare_jump = c.jump()

    # == FIGHT ==
    c.bind(fight_jump)

    c.exit_battle(won=True)
    post_exit = c.jump()

    # == CHECK ==
    c.bind(check_jump)

    c.dialogue_flavor(20, type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
    toriel_scare_jump_2 = c.jump()

    # == COMPLIMENT ==

    c.bind(compliment_jump)
    c.dialogue_flavor(30, type_sound="SND_TXT1.wav")
    # TODO: Add blush ribbit.
    c.wait(WaitTypes.DIALOGUE)

    # == TORIEL SCARE ==
    c.bind(toriel_scare_jump)
    c.bind(toriel_scare_jump_2)

    c.debug("Toriel scare")
    c.load_sprite(256, 192 // 4, "battle/spr_torieldisapprove")

    c.wait(WaitTypes.FRAMES, 40)

    c.move_in_frames(Target(TargetType.SPRITE, 0), -76, 0, 80)
    c.wait(WaitTypes.FRAMES, 120)
    c.set_animation(Target(TargetType.ENEMY, 0), "look_up")
    c.wait(WaitTypes.FRAMES, 80)
    c.set_animation(Target(TargetType.ENEMY, 0), "look_down")
    c.set_pos_in_frames(Target(TargetType.ENEMY, 0), -51, 93, 120)
    c.set_pos_in_frames(Target(TargetType.ENEMY, 0, 1), -51, 93, 120)
    c.wait(WaitTypes.FRAMES, 180)
    c.set_animation(Target(TargetType.SPRITE, 0), "up")
    c.stop_bgm()
    c.exit_battle(won=True)

    # == POST EXIT ==
    c.bind(post_exit)
    c.wait(WaitTypes.EXIT)
    c.start_bgm("mus_ruins.wav", True)
    c.clear_nav_tasks()
    c.set_pos(Target(TargetType.SPRITE, 0), 612, 127)
    c.set_animation(Target(TargetType.SPRITE, 0), "leftIdle")
