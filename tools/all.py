import os
import sys
from compileCutscenes import compile_cutscenes
from gmxToCfnt import compile_fonts
from jsonToCspr import compile_sprites
from jsonToRoom import compile_rooms
from pngToCbgf import compile_backgrounds
from compileBanks import compile_texts
import time


def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    # TODO: Add proper argparser
    force_cutscenes = False
    force_fonts = False
    force_sprites = False
    force_rooms = False
    force_bgs = False
    force_texts = False
    for arg in sys.argv:
        if arg == "--force" or arg == "-f":
            force_cutscenes = True
            force_fonts = True
            force_sprites = True
            force_rooms = True
            force_bgs = True
            force_texts = True
        elif arg == "--force-cutscenes" or arg == "-fc":
            force_cutscenes = True
        elif arg == "--force-fonts" or arg == "-ff":
            force_fonts = True
        elif arg == "--force-sprites" or arg == "-fs":
            force_sprites = True
        elif arg == "--force-rooms" or arg == "-fr":
            force_rooms = True
        elif arg == "--force-backgrounds" or arg == "-fb":
            force_bgs = True
        elif arg == "--force-texts" or arg == "-ft":
            force_texts = True

    compile_cutscenes(force_cutscenes)
    compile_fonts(force_fonts)
    compile_sprites(force_sprites)
    compile_rooms(force_rooms)
    compile_backgrounds(force_bgs)
    compile_texts(force_texts)

    # Hack to allow make to detect the changes
    with open("../nitrofs/stamp_file.txt", "w") as f:
        f.write(str(time.time()))


if __name__ == '__main__':
    main()
