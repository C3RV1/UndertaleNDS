import importlib.util
import sys
import CutsceneTypes
import binary
import os


def convert(input_path, output_path):
    c = CutsceneTypes.Cutscene(binary.BinaryWriter(open(output_path, "wb")))
    spec = importlib.util.spec_from_file_location("cutsceneImp", input_path)
    cutsceneImp = importlib.util.module_from_spec(spec)
    sys.modules["cutsceneImp"] = cutsceneImp
    spec.loader.exec_module(cutsceneImp)
    cutsceneImp.cutscene(c)
    c.end_cutscene()


def main():
    files = os.listdir("./cutscenes")
    for file in files:
        if not file.endswith(".py"):
            continue
        convert(os.path.join("./cutscenes", file),
                os.path.join("./cutscenes", os.path.splitext(file)[0] + ".cscn"))


if __name__ == '__main__':
    main()
