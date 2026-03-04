import typing
if typing.TYPE_CHECKING:
    from tools.CutsceneTypes import *
else:
    from CutsceneTypes import *


def cutscene(c: Cutscene):
    # TODO: Dialogue facial expressions.
    c.player_control(False)
    c.set_collider_enabled(1, False)
    c.play_sfx("snd_phone.wav", 0)
    c.dialogue_centered(10, "", 0, 0, "", "",
                        Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.cmp_flag(FlagOffsets.CINNAMON_BUTTERSCOTCH, "==", 2)
    chose_butterscotch = c.jump_if()

    # == CHOSE CINNAMON ==

    c.dialogue_centered(20, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    end = c.jump()
    c.bind(chose_butterscotch)

    # == CHOSE BUTTERSCOTCH ==

    c.dialogue_centered(21, "speaker/toriel", (256 - 50) // 2, (192 - 39) // 4 - 5,
                     "talkIdle", "talkTalk",
                        Target(TargetType.NULL),
                     "", "", type_sound="snd_txttor.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.bind(end)

    c.dialogue_centered(30, "", 0, 0, "", "",
                        Target(TargetType.NULL), "", "",
                        type_sound="SND_TXT1.wav")
    c.wait(WaitTypes.DIALOGUE)

    c.set_flag(FlagOffsets.PROGRESS, 10)
