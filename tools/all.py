import os
from compileCutscenes import compileCutscenes
from gmxToCfnt import compileFonts
from jsonToCspr import compileSprites
from jsonToRoom import compileRooms
from pngToCbgf import compileBackgrounds
import time


def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    compileCutscenes()
    compileFonts()
    compileSprites()
    compileRooms()
    compileBackgrounds()
    # Hack to allow make to detect the changes
    with open("../nitrofs/stamp_file.txt", "w") as f:
        f.write(str(time.time()))


if __name__ == '__main__':
    main()
