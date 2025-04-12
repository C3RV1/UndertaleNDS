import os
import sys
from compileCutscenes import compile_cutscenes
from gmxToCfnt import compile_fonts
from jsonToCspr import compile_sprites
from jsonToRoom import compile_rooms
from pngToCbgf import compile_backgrounds
import time


def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    force = False
    for arg in sys.argv:
        if arg == "--force":
            force = True
    compile_cutscenes(force)
    compile_fonts(force)
    compile_sprites(force)
    compile_rooms(force)
    compile_backgrounds(force)
    # Hack to allow make to detect the changes
    with open("../nitrofs/stamp_file.txt", "w") as f:
        f.write(str(time.time()))


if __name__ == '__main__':
    main()
