# UndertaleNDS
Undertale port for Nintendo DS (src only, no copyrighted material).

Note: A little bit of spaghetti code. Somewhaaat actually. Tbh, a lot. Too much spaghetti code. Don't judge me based on it.

Intro & Screen Name Clip: [https://youtu.be/p_8BHBVBwFQ](https://youtu.be/p_8BHBVBwFQ)

## Installation
To compile the game or use the rom, you will need to provide the `data.win` file from the
original UNDERTALE game. This file can be located at
`C:\Program Files (x86)\Steam\steamapps\common\Undertale\data.win` or
`C:\Program Files\Steam\steamapps\common\Undertale\data.win` for the Steam installation.

Once you have located this file, you should copy it to the root directory of the project
this file will be used to decrypt the copyrighted material. This way I make sure you
own a legitimate copy of the game before playing the NDS port. Say no to piracy!

You should download the AES-encrypted files (`Undertale.nds.aes`, `tools.zip.aes`,
`nitrofs.zip.aes`) from the latest release and also copy them to the project root.

Once you have all files at the root of the directory (`data.win`, `Undertale.nds.aes`, `tools.zip.aes`,
`nitrofs.zip.aes`) run `./decrypt_all.py` to decrypt the AES-encrypted files to the corresponding
project files (`Undertale.nds`, `tools`, `nitrofs`).

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
