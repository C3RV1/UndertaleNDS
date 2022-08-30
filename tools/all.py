import os
from compileCutscenes import compileCutscenes
from gmxToCfnt import compileFonts
from jsonToCspr import compileSprites
from jsonToRoom import compileRooms
from pngToCbgf import compileBackgrounds


def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    compileCutscenes()
    compileFonts()
    compileSprites()
    compileRooms()
    compileBackgrounds()


if __name__ == '__main__':
    main()
