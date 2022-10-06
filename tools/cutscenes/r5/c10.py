import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    # Dummy battle
    c.player_control(False)
    c.set_action(Target(TargetType.SPRITE, 1), "none")

    c.start_battle([Enemy(1, 15, 0, 2, BattleAttackIds.NONE)], 0, 61, 63, 134, 75)
    c.wait(WaitTypes.EXIT)

    # == LOAD BATTLE ==

    c.debug("Loading battle...")
    c.load_texture("battle/dummy_ruins")
    c.load_sprite(20, (192 - 104) // 2, 0)
    c.set_scale(Target(TargetType.SPRITE, 0), 1.5, 1.5)
    c.start_bgm("mus_prebattle1.wav", True)

    c.wait(WaitTypes.ENTER)

    c.start_dialogue_battle(10, 100, 192 // 4 + 20,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    # == BATTLE LOOP ==

    loop_start = c.debug("Battle loop!")

    c.battle_action()
    c.wait(WaitTypes.BATTLE_ACTION)

    c.cmp_enemy_hp(0, "<", 15)
    fight_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.ACT + 1)
    talk_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_ACTION, "==", BtlActionOff.FLEE)
    flee_jump = c.jump_if()

    c.mod_flag(FlagOffsets.BATTLE_FLAGS, 1)
    c.cmp_flag(FlagOffsets.BATTLE_FLAGS, ">=", 8)
    bored_dummy_jump = c.jump_if()

    c.cmp_flag(FlagOffsets.BATTLE_FLAGS, "==", 1)
    jump_idle_1 = c.jump_if()

    # == IDLE 2 ==

    c.start_dialogue_battle(12, 100, 192 // 4 + 20,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.jump(dst=loop_start)

    # == IDLE 1 ==

    c.bind(jump_idle_1)

    c.start_dialogue_battle(11, 100, 192 // 4 + 20,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.jump(dst=loop_start)

    # == FIGHT ==

    c.bind(fight_jump)
    c.debug("Dummy killed")
    c.set_flag(FlagOffsets.DUMMY, 0)
    c.set_animation(Target(TargetType.SPRITE, 0), "hurt")

    c.start_dialogue_battle(20, 100, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.exit_battle(won=True)
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

    c.bind(talk_jump)
    c.debug("Talked to dummy")
    c.set_flag(FlagOffsets.DUMMY, 1)

    c.start_dialogue_battle(40, 100, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.exit_battle(won=True)

    c.wait(WaitTypes.EXIT)
    c.wait(WaitTypes.ENTER)

    c.start_dialogue(45, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
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
    c.wait(WaitTypes.ENTER)

    c.start_dialogue(50, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
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

    c.start_dialogue_battle(60, 100, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -200, 120)

    c.start_dialogue_battle(61, 100, 192 // 4,
                            Target(TargetType.NULL), "", "",
                            type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.FRAMES, value=120)
    c.wait(WaitTypes.DIALOGUE)

    c.exit_battle(won=True)
    c.wait(WaitTypes.EXIT)
    c.wait(WaitTypes.ENTER)

    c.start_dialogue(65, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4,
                     "talkIdle", "talkTalk",
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

    c.set_animation(Target(TargetType.SPRITE, 0), "moveUp")
    c.move_in_frames(Target(TargetType.SPRITE, 0), 0, -30, 20)
    c.wait(WaitTypes.FRAMES, 20)
    c.unload_sprite(0)
    c.unload_texture(0)
