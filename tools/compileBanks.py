import binary
import os

def find_largest_match(window: list, data: list, max_length: int):
    largest_match_start = -1
    largest_match_length = 3
    match_start = len(window) - 1
    match_length = 0
    for i, e in enumerate(window):
        if match_length == len(data) or match_length >= max_length:
            break
        if e == data[match_length]:
            match_length += 1
            continue
        elif match_length >= largest_match_length:
            largest_match_start = match_start
            largest_match_length = match_length
        match_start = len(window) - 1 - i
        match_length = 0
    if match_length >= largest_match_length:
        largest_match_start = match_start
        largest_match_length = match_length
    return (largest_match_start, largest_match_length)


def lz77_encode(data: bytes):
    data_lst = list(data)
    window = list()
    window_size = 0x1000

    out_blocks = []
    while data_lst:
        match_start, match_length = find_largest_match(window, data_lst, 0xF + 3)
        if match_start == -1:
            out_blocks.append(data_lst[0])
            window.append(data_lst[0])
            l = 1
        else:
            out_blocks.append((match_start, match_length))
            window.extend(data_lst[:match_length])
            l = match_length
        data_lst = data_lst[l:]
        if len(window) > window_size:
            bytes_to_remove = len(window) - window_size
            window = window[bytes_to_remove:]

    out = binary.BinaryWriter()
    i = 0
    while i < len(out_blocks):
        flags = 0
        for j, block in enumerate(out_blocks[i:i+8]):
            if isinstance(block, tuple):
                flags += 1 << (7 - j)
        out.write_uint8(flags)
        for block in out_blocks[i:i+8]:
            if isinstance(block, tuple):
                disp, length = block
                disp -= 1
                if disp > 0xFFF:
                    raise ValueError()
                byte1 = ((length - 3) & 0xF) << 4
                byte1 += (disp & 0xF00) >> 8
                out.write_uint8(byte1) 
                byte2 = disp & 0xFF
                out.write_uint8(byte2) 
            else:
                out.write_uint8(block)
        i += 8
    return out.data


def compile_bank(bank_folder: str, path_dest: str, extension: str, force: bool = False):
    table = []
    data = bytes()

    convert = force
    if os.path.isfile(path_dest):
        dst_time = os.path.getmtime(path_dest)
    else:
        dst_time = 0

    for root, _, files in os.walk(bank_folder):
        for file in files:
            path = os.path.join(root, file)
            if not path.endswith(extension):
                continue
            src_time = os.path.getmtime(path)
            if src_time > dst_time or force:
                convert = True
            with open(path, "rb") as f:
                data_file = f.read()
            path = "/".join(path.split("/")[1:])
            table.append((path, len(data), len(data_file)))
            data += data_file

    if not convert:
        return
    print(f"Created table {len(table)}:")
    for te in table:
        print(te)
    all_data = binary.BinaryWriter()
    all_data.write_uint32(len(table))
    for name, start, length in table:
        if len(name) > 32:
            raise ValueError(name)
        all_data.write_string(name, pad=b'\0', size=32)
        all_data.write_uint32(start)
        all_data.write_uint32(length)
    all_data.write(data)
    file_full = all_data.data

    print("Compressing...")
    file_compressed = lz77_encode(file_full)
    original_size = len(file_full)
    compressed_size = len(file_compressed)
    print(f"Original size: {original_size}")
    print(f"Compressed size: {compressed_size}")
    print(f"Ratio: {compressed_size*100/original_size}")

    wtr = binary.BinaryWriter(open(path_dest, "wb"))
    wtr.write(b"CBNK")
    file_size_pos = wtr.tell()
    wtr.write_uint32(0)
    wtr.write_uint32(1) # Version
    compression_header = (1 << 4) + (original_size << 8)
    wtr.write_uint32(compression_header)
    wtr.write(file_compressed)

    size = wtr.tell()
    wtr.seek(file_size_pos)
    wtr.write_uint32(size)
    wtr.close()

def compile_texts(force: bool = False):
    compile_bank("txt", "../nitrofs/txts.cbnk", ".txt", force=force)

if __name__ == "__main__":
    compile_texts(True)
        
