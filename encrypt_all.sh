python3 tools/all.py
make
python3 -m pip install pycryptodome
python3 tools/encrypt_files.py ../Undertale.nds ../Undertale.nds.aes
python3 tools/encrypt_files.py ../nitrofs ../nitrofs.zip.aes
python3 tools/encrypt_files.py ../tools ../tools.zip.aes