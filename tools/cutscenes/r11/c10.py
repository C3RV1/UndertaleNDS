import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    c.player_control(False)
    c.set_animation(Target(TargetType.PLAYER), "rightIdle")
    c.set_collider_enabled(2, False)
    c.play_sfx("snd_phone.wav", 0)
    c.dialogue_centered(10, "", 0, 0, "", "",
                        Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.cmp_flag(FlagOffsets.CINNAMON_BUTTERSCOTCH, "==", 1)
    chosen_cinnamon_before = c.jump_if()
    c.cmp_flag(FlagOffsets.CINNAMON_BUTTERSCOTCH, "==", 2)
    chosen_butterscotch_before = c.jump_if()

    # == FIRST TIME ==
    c.dialogue_centered(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.cmp_flag(FlagOffsets.DIALOGUE_OPTION, "==", 0)  # cinnamon is 0
    after_intro = c.jump()

    # == CHOSEN CINNAMON BEFORE ==
    c.bind(chosen_cinnamon_before)
    c.dialogue_centered(21, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.cmp_flag(FlagOffsets.DIALOGUE_OPTION, "==", 0)  # cinnamon is yes
    guessed_correctly = c.jump_if()
    guessed_incorrectly = c.jump_if_not()

    # == CHOSEN BUTTERSCOTCH BEFORE ==
    c.bind(chosen_butterscotch_before)

    c.dialogue_centered(22, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)
    c.cmp_flag(FlagOffsets.DIALOGUE_OPTION, "==", 1)  # cinnamon is no
    guessed_incorrectly2 = c.jump_if()

    # == GUESSED CORRECTLY ==
    c.bind(guessed_correctly)

    c.dialogue_centered(23, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    after_intro2 = c.jump()

    # == GUESSED INCORRECTLY ==
    c.bind(guessed_incorrectly)
    c.bind(guessed_incorrectly2)

    c.dialogue_centered(24, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    # == END ==
    c.bind(after_intro)
    c.bind(after_intro2)
    butterscotch_jump = c.jump_if_not()

    c.set_flag(FlagOffsets.CINNAMON_BUTTERSCOTCH, 1)

    end_jump = c.jump()
    c.bind(butterscotch_jump)

    c.set_flag(FlagOffsets.CINNAMON_BUTTERSCOTCH, 2)

    c.bind(end_jump)
    c.dialogue_centered(30, "", 0, 0, "", "",
                        Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.set_flag(FlagOffsets.PROGRESS, 9)
