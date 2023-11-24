import os
import shutil
from tkinter import filedialog, messagebox
try:
    from Crypto.Cipher import AES
    from Crypto.Hash import SHA256
except ImportError as e:
    messagebox.showerror(
        title="Couldn't find requirement.",
        message="Couldn't import pycryptodome,"
                "required for patching."
    )
    raise e


def generate_patch_key(k_f):
    def xor(a: bytes, b: bytes) -> bytes:
        return bytes([av ^ bv for av, bv in zip(a, b)])

    key = b"\0" * 32
    hash_sha = SHA256.new()
    while k_f_frag := k_f.read(32):
        if len(k_f_frag) != 32:
            break
        key = xor(k_f_frag, key)
        hash_sha.update(k_f_frag)
    if hash_sha.hexdigest() != "d8c7f0cdbbc448931de95cc3a6ff62588e57097c7a9f147eb31aba7a0624e81e":
        print("Game version not supported (hash does not match)")
        return False, b""
    return True, key


def patch(input_f, key, output_f):
    i_f = open(input_f, "rb")
    o_f = open(output_f, "wb")

    nonce, tag, ciphertext = [i_f.read(x) for x in (16, 16, -1)]
    cipher = AES.new(key, AES.MODE_EAX, nonce)
    data = cipher.decrypt_and_verify(ciphertext, tag)
    o_f.write(data)

    i_f.close()
    o_f.close()


def main():
    key_f_name = filedialog.askopenfilename(
        filetypes=[("Undertale Data File", "*.win")],
        title="Choose Undertale data.win file"
    )
    with open(key_f_name, "rb") as key_f:
        valid, key = generate_patch_key(key_f)

    if not valid:
        messagebox.showerror(
            title="Unsupported .win",
            message="This version of data.win is not supported.\n"
                    "The only version supported is Windows Steam Undertale."
        )
        return

    patch_files = filedialog.askopenfilenames(
        filetypes=[("Patch files", "*.patch")],
        title="Choose all .patch files"
    )
    for patch_file in patch_files:
        os.chdir(os.path.dirname(os.path.abspath(__file__)))
        if patch_file.endswith(".zip.patch"):
            zip_path = os.path.splitext(patch_file)[0]
            dir_path = os.path.splitext(zip_path)[0]
            patch(patch_file, key, zip_path)
            shutil.unpack_archive(zip_path, dir_path, 'zip')
            os.remove(zip_path)
        else:
            out_path = os.path.splitext(patch_file)[0]
            patch(patch_file, key, out_path)
    messagebox.showinfo(
        title="Completed successfully",
        message="Patching completed successfully!"
    )


if __name__ == '__main__':
    main()

