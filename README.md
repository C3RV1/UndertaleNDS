# UndertaleNDS
**This project is currently halted. Progress will be resumed whenever
possible.**

UndertaleNDS is a port of the famous 2015 game by Toby Fox for the NintendoDS.
It aims to recreate somewhat accurately the mechanics, graphics and gameplay of
the original, while adapting them to the destination platform as best as
possible.

If you want to get an idea of what the game looks like, you can check out the
UndertaleNDS - 10 minute gameplay:
[https://youtu.be/n27m40_RAGQ](https://youtu.be/n27m40_RAGQ),
although it is quite outdated at the moment.

If you want to play the game on your own console/emulator, follow the
[[Creating the ROM]] instructions. If you wish, however, to tinker with the
source code, or make your own modifications, go to [[Building from source]].

**NOTE: As this repository only contains the original source code, and no
copyrighted material from the game, the first thing you'll need is to have
bought the game. Then, get the `data.win` file from the game files (in Steam
and Windows, located in
`C:/Program Files/Steam/steamapps/common/Undertale/data.win`). This is the file
you'll need for creating the ROM or getting the assets for building the game.**

*(Also, the compatibility with a `data.win` file from any other version that
the one mentioned above is not guaranteed.)*

## Creating the ROM
Download the following files from the latest release: `patcher.exe`,
`Undertale.nds.patch`, `nitrofs.zip.patch`, `tools.zip.patch`.
Then, run the `.exe` file, which is just a bundled python script.

Alternatively, you can find the source code that generated the `.exe` file at
the root of the repository, named `patch.py`. If you prefer to run the python
script instead, make sure to have `tkinter` installed and all the requirements
by running:
```sh
python -m pip install -r requirements.txt
```

Once you've run the `patcher.exe` file, or the python script, just provide the
files the program asks for, and it'll create the `.nds` rom automatically.

Note: If you wish to play the game on an emulator, the best one to do so is,
at the moment, melonDS 0.9.1. Versions after 0.9.1 require you to set up a DLDI
system for the game to be able to save, so keep that in mind if you decide to
play in a more recent version.

## Building from source

To build the project from the source code, you'll need to have installed the
[BlocksDS](https://blocksds.skylyrac.net/) development SDK for the NintendoDS.
Just follow the instructions from the webpage linked.

Then, proceed by cloning the
repository, and follow the steps in [[Creating the ROM]] to create the
`tools` and `nitrofs` folders you'll need to compile the game. Running the
patcher will also create a `.nds` file, but you can ignore it: it will be
replaced by a new file once you compile the project.
The reason you create these folders this way is that they contain copyrighted
data from the game, and as such can only be created by using the `data.win`
file.

Then, you can proceed by modifying any of the assets/code from the project.
The code is structured as such:
- The `source` and `include` directories contain the C++ code in which the game
engine is written. It contains an `Engine` subdirectory, which is used for
handling the interaction with the hardware, and all other directories form the
actual game logic.
- The `tools` directory contains all intermediate files which will be converted
to game assets. These are, for example:
   - `.png` images for the backgrounds in the `bg` subdirectory.
   - `.json`/`.png` for sprites (the first file has metadata, like the number
  of frames, the animations..., while the second one has the actual frames) in
  the `spr` subdirectory.
   - `.json` for rooms, which specify the possible transitions to other
  rooms, the colliders in the room, the sprites to be loaded when the room is
  entered, the music to be played... in the `rooms` subdirectory.
  - `.py` scripts (which compile to a binary stream) for cutscenes, located
  in the `cutscene` subdirectory.
  - `.txt` for all texts in the game, in the `txt` subdirectory.
  - `.gmx`/`.png` for fonts, in the `fnt` subdirectory.
- The `nitrofs` directory contains all the compiled data from the `tools`
directory, in a format that the game engine supports. The only data specific
to this directory is the audio from the game as `.wav` files,
located in the `z_audio` subdirectory.

Once you've made any desired changes to all these assets, you can build the
game by running `python tools/all.py`, which will convert all the data files
in the `tools` directory to the corresponding files in the `nitrofs` directory.
Note that it is normal for this step to generate some errors, specifically when
trying to process rooms which are not yet finished.

After compiling the assets, build the rom by running `make`.

## Credits
Toby Fox - Original game
Cervi - DS Port
