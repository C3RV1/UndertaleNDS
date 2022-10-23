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

Once you have located this file, you should download the UPS patches from the release tab
and apply them on `data.win` (you can use any UPS utility,
[Tsukuyomi UPS](https://www.romhacking.net/utilities/519/) for example).
After applying the patches, you can boot `Undertale.nds`
directly to run the game or decompress `tools.zip` and `nitrofs.zip` to the root directory
of the project if you wish to build it and make any modifications to it.

All this should result in the corresponding
project files: `Undertale.nds`, `tools` (directory) and `nitrofs` (directory).

## Compiling
Note: Make sure to follow the installation instructions, which will
extract `nitrofs` and `tools`, needed for compiling.

When you make any changes to the assets the game uses (located in `tools/spr`, `tools/bg`, `tools/cutscenes`,
`tools/rooms`, `tools/fnt`) you should run `python3 tools/all.py` in order to recompile
all these files to their corresponding rom files.

Then, you should run `make` to build the rom. Make sure to have
[devkitarm](https://devkitpro.org/wiki/Getting_Started) installed.

## Credits
Toby Fox - Original Game  
Cervi - DS Port
