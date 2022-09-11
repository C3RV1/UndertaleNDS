import sys

from Crypto.Cipher import AES
from Crypto.Hash import SHA256
import os
import shutil


def main(input_f, key_f, output_f):
    i_f = open(input_f, "rb")
    k_f = open(key_f, "rb")
    o_f = open(output_f, "wb")

    def xor(a: bytes, b: bytes) -> bytes:
        return bytes([av ^ bv for av, bv in zip(a, b)])

    key = b"\0" * 32
    hash_sha = SHA256.new()
    while k_f_frag := k_f.read(32):
        if len(k_f_frag) != 32:
            break
        key = xor(k_f_frag, key)
        hash_sha.update(k_f_frag)
    k_f.close()
    if hash_sha.hexdigest() != "d8c7f0cdbbc448931de95cc3a6ff62588e57097c7a9f147eb31aba7a0624e81e":
        print("Game version not supported (hash does not match)")
        return

    nonce, tag, ciphertext = [i_f.read(x) for x in (16, 16, -1)]
    cipher = AES.new(key, AES.MODE_EAX, nonce)
    data = cipher.decrypt_and_verify(ciphertext, tag)
    o_f.write(data)
    i_f.close()
    o_f.close()


if __name__ == '__main__':
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if os.path.splitext(sys.argv[2])[1] == "":
        main(sys.argv[1], "../data.win", sys.argv[2] + ".zip")
        shutil.unpack_archive(sys.argv[2] + ".zip", sys.argv[2], 'zip')
        os.remove(sys.argv[2] + ".zip")
    else:
        main(sys.argv[1], "../data.win", sys.argv[2])
