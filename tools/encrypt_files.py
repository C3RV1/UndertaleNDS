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

    cipher = AES.new(key, AES.MODE_EAX)
    ciphertext, tag = cipher.encrypt_and_digest(i_f.read())

    [o_f.write(x) for x in (cipher.nonce, tag, ciphertext)]
    i_f.close()
    o_f.close()


if __name__ == '__main__':
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    if os.path.isdir(sys.argv[1]):
        print(sys.argv[1] + ".zip")
        shutil.make_archive(sys.argv[1], 'zip', sys.argv[1])
        main(sys.argv[1] + ".zip", "../data.win", sys.argv[2])
        os.remove(sys.argv[1] + ".zip")
    else:
        main(sys.argv[1], "../data.win", sys.argv[2])
