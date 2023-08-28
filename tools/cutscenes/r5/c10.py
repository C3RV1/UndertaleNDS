import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    # Dummy battle
    c.player_control(False)
    c.set_action(Target(TargetType.SPRITE, 1), "none")

    c.start_battle([Enemy(1, 15, -5, 0, 2, BattleAttackIds.NONE)], 0, 61, 63, 134, 75)
    c.wait(WaitTypes.EXIT)

    # == LOAD BATTLE ==

    c.debug("Loading battle...")
    c.load_texture("battle/dummy_ruins")
    c.load_sprite(30, (192 - 52) // 2, 0)
    # c.set_scale(Target(TargetType.SPRITE, 0), 1.5, 1.5)
    c.start_bgm("mus_prebattle1.wav", True)

    c.wait(WaitTypes.ENTER)
    c.battle_action(0)

    # == BATTLE LOOP ==

    loop_start = c.debug("Battle loop (post action)!")

    c.wait(WaitTypes.BATTLE_ACTION)

    c.cmp_enemy_hp(0, "<", 15)
    fight_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.ACT)
    check_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.ACT + 1)
    talk_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.FLEE)
    flee_jump = c.jump_if()

    continue_battle = c.mod_flag(FlagOffsets.BATTLE_FLAGS, 1)
    c.cmp_flag(FlagOffsets.BATTLE_FLAGS, ">=", 8)
    bored_dummy_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_FLAGS, "==", 1)
    jump_idle_1 = c.jump_if()

    # == IDLE 2 ==

    c.dialogue_left_align(12, 90, 192 // 4 + 20,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.battle_action(1)
    c.jump(dst=loop_start)

    # == IDLE 1 ==

    c.bind(jump_idle_1)

    c.dialogue_left_align(12, 90, 192 // 4 + 20,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.battle_action(2)
    c.jump(dst=loop_start)

    # == FIGHT ==

    c.bind(fight_jump)
    c.debug("Dummy killed")
    c.set_flag(FlagOffsets.DUMMY, 0)
    c.set_animation(Target(TargetType.SPRITE, 0), "hurt")

    c.dialogue_left_align(20, 90, 192 // 4,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.stop_bgm()

    c.exit_battle(won=True)
    c.wait(WaitTypes.EXIT)

    c.start_bgm("mus_ruins.wav", True)

    # We have killed it, so unload it
    c.unload_sprite(-1)
    c.unload_texture(-1)

    c.wait(WaitTypes.ENTER)

    c.dialogue_centered(25, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    post_fight_jump = c.jump()

    # == CHECK ==

    c.bind(check_jump)
    c.debug("Check dummy")

    c.dialogue_left_align(30, 90, 192 // 4,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav", main_screen=False)
    c.wait(WaitTypes.DIALOGUE)

    c.jump(continue_battle)

    # == TALK ==

    c.bind(talk_jump)
    c.debug("Talked to dummy")
    c.set_flag(FlagOffsets.DUMMY, 1)

    c.dialogue_left_align(40, 90, 192 // 4,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.stop_bgm()
    c.exit_battle(won=True)

    c.wait(WaitTypes.EXIT)
    c.start_bgm("mus_ruins.wav", True)
    c.wait(WaitTypes.ENTER)

    c.dialogue_centered(45, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    post_talk_jump = c.jump()

    # == FLEE ==

    c.bind(flee_jump)
    c.debug("Flee from dummy")
    c.set_flag(FlagOffsets.DUMMY, 2)

    c.exit_battle(won=False)
    c.wait(WaitTypes.EXIT)
    c.start_bgm("mus_ruins.wav", True)
    c.wait(WaitTypes.ENTER)

    c.dialogue_centered(50, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
                        Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    post_flee_jump = c.jump()

    # == BORED ==

    c.bind(bored_dummy_jump)
    c.debug("Dummy got bored")
    c.set_flag(FlagOffsets.DUMMY, 3)

    c.dialogue_left_align(12, 90, 192 // 4,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -120, 90)

    c.dialogue_left_align(61, 90, 192 // 4,
                          Target(TargetType.NULL), "", "",
                          type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.FRAMES, value=90)
    c.wait(WaitTypes.DIALOGUE)

    c.stop_bgm()
    c.exit_battle(won=True)
    c.wait(WaitTypes.EXIT)
    c.start_bgm("mus_ruins.wav", True)
    c.wait(WaitTypes.ENTER)

    c.dialogue_centered(65, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "what", "what",
                        Target(TargetType.SPRITE, 0),
                     "downIdle", "downTalk",
                        type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    # == BRANCH MERGE ==

    c.bind(post_fight_jump)
    c.bind(post_talk_jump)
    c.bind(post_flee_jump)

    c.debug("Branches merged")
    c.set_flag(FlagOffsets.PROGRESS, 5)
    c.set_collider_enabled(1, False)

    c.set_animation(Target(TargetType.SPRITE, 0), "upMove")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -30, 20)
    c.wait(WaitTypes.FRAMES, 20)
    c.unload_sprite(0)
    c.unload_texture(0)
