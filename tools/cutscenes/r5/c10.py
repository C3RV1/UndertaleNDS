import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import Cutscene, Target, TargetType, WaitTypes, Enemy, AttackOffset
else:
    from CutsceneTypes import Cutscene, Target, TargetType, WaitTypes, Enemy, AttackOffset


def cutscene(c: Cutscene):
    # Dummy battle
    c.player_control(False)

    c.start_battle([Enemy(0, 15)], 0, 61, 63, 134, 75)
    c.wait(WaitTypes.EXIT)

    # == LOAD BATTLE ==

    c.debug("Loading battle...")
    c.load_texture("battle/dummy_ruins")
    c.load_sprite(30, (192 - 104) // 2, 0)

    c.wait(WaitTypes.ENTER)

    c.start_dialogue_battle(10, 128 + 20, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    # == BATTLE LOOP ==

    loop_start = c.debug("Battle loop!")

    c.wait_battle_action(0, [[0, 0], [0, 0]])
    c.wait(WaitTypes.BATTLE_ACTION)

    c.cmp_enemy_hp(0, "<", 15)
    fight_jump = c.jump_if()

    c.cmp_battle_action(AttackOffset.ACT + 1)
    talk_jump = c.jump_if()

    c.cmp_battle_action(AttackOffset.FLEE)
    flee_jump = c.jump_if()

    c.mod_flag(230, 1)
    c.cmp_flag(230, ">=", 8)
    bored_dummy_jump = c.jump_if()

    c.jump(dst=loop_start)

    # == FIGHT ==

    c.debug("Dummy killed")
    c.set_flag(1, 0)

    c.bind(fight_jump)
    c.start_dialogue_battle(20, 128 + 20, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.exit_battle()
    c.wait(WaitTypes.EXIT)

    # We have killed it, so unload it
    c.unload_sprite(1)
    c.unload_texture(1)

    c.wait(WaitTypes.ENTER)

    c.start_dialogue(25, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                     Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk",
                     type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    post_fight_jump = c.jump()

    # == TALK ==

    c.debug("Talked to dummy")
    c.set_flag(1, 1)

    c.bind(talk_jump)
    c.start_dialogue_battle(40, 128 + 20, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.exit_battle()

    post_talk_jump = c.jump()

    # == FLEE ==

    c.debug("Flee from dummy")
    c.set_flag(1, 2)

    c.bind(flee_jump)
    c.exit_battle()
    c.wait(WaitTypes.EXIT)
    c.wait(WaitTypes.ENTER)
    post_flee_jump = c.jump()

    # == BORED ==

    c.debug("Dummy got bored")
    c.set_flag(1, 3)

    c.bind(bored_dummy_jump)
    c.start_dialogue_battle(60, 128 + 20, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.exit_battle()

    post_bored_jump = c.jump()

    c.bind(post_fight_jump)
    c.bind(post_talk_jump)
    c.bind(post_bored_jump)
    c.bind(post_flee_jump)

    c.debug("Branches merged")

    c.set_animation(Target(TargetType.SPRITE, 0), "moveUp")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -30, 20)
    c.wait(WaitTypes.FRAMES, 20)
    c.unload_sprite(0)
    c.unload_texture(0)
