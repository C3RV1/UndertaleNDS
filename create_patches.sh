# python3 tools/all.py
# make
# python3 -m pip install pycryptodome
python3 tools/create_patch_file.py ../Undertale.nds ../Undertale.nds.patch
python3 tools/create_patch_file.py ../nitrofs ../nitrofs.zip.patch
python3 tools/create_patch_file.py ../tools ../tools.zip.patch
