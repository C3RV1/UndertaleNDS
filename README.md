# UndertaleNDS
Undertale port for Nintendo DS (src only, no copyrighted material).

Note: A bit of spaghetti code. Somewhaaat, actually. Tbh, a lot. Too much spaghetti code.
Don't judge me :)

UndertaleNDS - 10 minute gameplay: [https://youtu.be/n27m40_RAGQ](https://youtu.be/n27m40_RAGQ)

## Installation
I recommend you run the game on MelonDS if you're using an emulator. It yiels
the best results in terms of audio and performance.

To compile the game or use the rom, you will need to provide the `data.win` file from the
original UNDERTALE game. This file can be located at
`C:\Program Files (x86)\Steam\steamapps\common\Undertale\data.win` or
`C:\Program Files\Steam\steamapps\common\Undertale\data.win` for the Steam installation.

Once you have located this file, you should copy it to the root directory of the project.
This file will be used to get the copyrighted material. This way I make sure you
own a legitimate copy of the game before playing the NDS port. Say no to piracy!

You should download the patch files (`Undertale.nds.patch`, `tools.zip.patch`,
`nitrofs.zip.patch`) from the latest release and also copy them to the project root.

Once you have all files at the root of the directory (`data.win`, `Undertale.nds.patch`, `tools.zip.patch`,
`nitrofs.zip.patch`) run `./patch_all.sh`.

All this should result in the corresponding
project files: `Undertale.nds`, `tools` (directory) and `nitrofs` (directory).

## Compiling
Note: Make sure to follow the installation instructions, which will
extract `nitrofs` and `tools`, needed for compiling.

When you make any changes to the assets the game uses (located in `tools/spr`, `tools/bg`, `tools/cutscenes`,
`tools/rooms`, `tools/fnt`) you should run `python3 tools/all.py` in order to recompile
all these files to their corresponding rom files. Some other information used by the game is:
- Dialogues, in `nitrofs/data/dialogue`
- Cell calls, in `nitrofs/data/cell`
- Battle act texts, in `nitrofs/data/battle_act_txt`
- Enemy names, in `nitrofs/data/enemies`
- Item names and descriptions, in `nitrofs/data/items`
- Room names, in `nitrofs/data/room_names`
- Other .txt files in `nitrofs/data`

Then, you should run `make` to build the rom. Make sure to have
[devkitarm](https://devkitpro.org/wiki/Getting_Started) installed.

## Credits
Toby Fox - Original Game  
Cervi - DS Port
