import struct
import typing
from io import BytesIO, SEEK_SET, SEEK_CUR, SEEK_END
from typing import *

__all__ = ["BinaryWriter", "BinaryReader", "BinaryEditor",
           "SEEK_SET", "SEEK_END", "SEEK_CUR"]


class _BaseBinaryWrapper:
    def __init__(self, stream: Union[typing.BinaryIO, bytes] = b""):
        if isinstance(stream, bytes) or isinstance(stream, bytearray):
            self.stream = BytesIO(stream)
        else:
            self.stream = stream

    # Wrappings:
    def close(self) -> None:
        return self.stream.close()

    def flush(self) -> None:
        return self.stream.flush()

    def read(self, n: int = -1) -> bytes:
        return self.stream.read(n)

    def readable(self) -> bool:
        return self.stream.readable()

    def readline(self, limit: int = -1) -> AnyStr:
        return self.stream.readline(limit)

    def readlines(self, hint: int = -1) -> List[AnyStr]:
        return self.stream.readlines(hint)

    def write(self, s: Union[bytes, bytearray]) -> int:
        return self.stream.write(s)

    def writable(self) -> bool:
        return self.stream.writable()

    def writelines(self, lines: Iterable[AnyStr]) -> None:
        self.stream.writelines(lines)

    def seek(self, offset: int, whence: int = 0) -> int:
        return self.stream.seek(offset, whence)

    def seekable(self) -> bool:
        return self.stream.seekable()

    def tell(self) -> int:
        return self.stream.tell()

    def fileno(self) -> int:
        return self.stream.fileno()

    def __enter__(self):
        self.stream.__enter__()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stream.__exit__(exc_type, exc_val, exc_tb)

    # helper functions

    def readall(self):
        self.stream.seek(0)
        return self.stream.read()

    def getvalue(self):
        if isinstance(self.stream, BytesIO):
            return self.stream.getvalue()
        pos = self.stream.tell()
        ret = self.readall()
        self.stream.seek(pos)
        return ret

    def align(self, alignment=4):
        if offset := (self.tell() % alignment):
            self.seek(self.tell() + alignment - offset)

    # some properties to make it more like the old BinaryReader
    @property
    def data(self):
        return self.getvalue()

    @property
    def c(self):
        return self.tell()

    @c.setter
    def c(self, value):
        self.seek(value)

    def __len__(self):
        return len(self.getvalue())


class BinaryReader(_BaseBinaryWrapper):
    # Read types
    def read_struct(self, fmt) -> Optional[Tuple[Any]]:
        chunksize = struct.calcsize("<" + fmt)
        chunk = self.read(chunksize)
        if len(chunk) != chunksize:
            return None
        return struct.unpack("<" + fmt, chunk)

    def read_char(self) -> Optional[AnyStr]:
        val = self.read_struct("c")
        return val[0] if val else None

    def read_bool(self) -> Optional[bool]:
        val = self.read_struct("?")
        return val[0] if val else None

    def read_byte(self) -> Optional[int]:
        val = self.read_struct("b")
        return val[0] if val else None

    def read_ubyte(self) -> Optional[int]:
        val = self.read_struct("B")
        return val[0] if val else None

    def read_short(self) -> Optional[int]:
        val = self.read_struct("h")
        return val[0] if val else None

    def read_ushort(self) -> Optional[int]:
        val = self.read_struct("H")
        return val[0] if val else None

    def read_int(self) -> Optional[int]:
        val = self.read_struct("i")
        return val[0] if val else None

    def read_uint(self) -> Optional[int]:
        val = self.read_struct("I")
        return val[0] if val else None

    def read_long(self) -> Optional[int]:
        val = self.read_struct("l")
        return val[0] if val else None

    def read_ulong(self) -> Optional[int]:
        val = self.read_struct("L")
        return val[0] if val else None

    def read_longlong(self) -> Optional[int]:
        val = self.read_struct("q")
        return val[0] if val else None

    def read_ulonglong(self) -> Optional[int]:
        val = self.read_struct("Q")
        return val[0] if val else None

    def read_float(self) -> Optional[float]:
        val = self.read_struct("f")
        return val[0] if val else None

    def read_double(self) -> Optional[float]:
        val = self.read_struct("d")
        return val[0] if val else None

    def read_string(self, size: Optional[int] = None, encoding: Optional[str] = "shift_jis", pad=b"\0"):
        if size:
            if encoding:
                return self.read(size).split(pad)[0].split(b"\0")[0].decode(encoding)
            else:
                return self.read(size).split(pad)[0].split(b"\0")[0]
        else:
            ret = b""
            while True:
                c = self.read(1)
                if c is None or c == pad:
                    break
                ret += c
            if encoding:
                return ret.decode(encoding)
            else:
                return ret

    def read_int24(self) -> Optional[int]:  # Little endian only
        chunk = self.read(3)
        if len(chunk) != 3:
            return None
        return struct.unpack('<i', chunk + (b'\0' if chunk[2] < 128 else b'\xff'))[0]

    def read_uint24(self) -> Optional[int]:  # Little endian only
        chunk = self.read(3)
        if len(chunk) != 3:
            return None
        return struct.unpack('<I', chunk + b"\0")[0]

    #  Arrays
    def read_struct_array(self, n: int, fmt):
        return [self.read_struct(fmt) for _ in range(n)]

    def read_char_array(self, n: int) -> Optional[List[AnyStr]]:
        return list(self.read_struct(f"{n}c"))

    def read_bool_array(self, n: int) -> Optional[List[bool]]:
        return list(self.read_struct(f"{n}?"))

    def read_byte_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}b"))

    def read_ubyte_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}B"))

    def read_short_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}h"))

    def read_ushort_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}H"))

    def read_int_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}i"))

    def read_uint_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}I"))

    def read_long_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}l"))

    def read_ulong_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}L"))

    def read_longlong_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}q"))

    def read_ulonglong_array(self, n: int) -> Optional[List[int]]:
        return list(self.read_struct(f"{n}Q"))

    def read_float_array(self, n: int) -> Optional[List[float]]:
        return list(self.read_struct(f"{n}f"))

    def read_double_array(self, n: int) -> Optional[List[float]]:
        return list(self.read_struct(f"{n}d"))

    def read_string_array(self, n: int, size: Optional[int] = None, encoding: Optional[str] = "shift_jis", pad=b"\0"):
        return [self.read_string(size, encoding, pad) for _ in range(n)]

    def read_int24_array(self, n: int) -> Optional[List[float]]:
        return [self.read_int24() for _ in range(n)]

    def read_uint24_array(self, n: int) -> Optional[List[float]]:
        return [self.read_uint24() for _ in range(n)]

    # Aliasses
    def read_int8(self) -> Optional[int]:
        return self.read_byte()

    def read_int16(self) -> Optional[int]:
        return self.read_short()

    def read_int32(self) -> Optional[int]:
        return self.read_int()

    def read_int64(self) -> Optional[int]:
        return self.read_longlong()

    def read_uint8(self) -> Optional[int]:
        return self.read_ubyte()

    def read_uint16(self) -> Optional[int]:
        return self.read_ushort()

    def read_uint32(self) -> Optional[int]:
        return self.read_uint()

    def read_uint64(self) -> Optional[int]:
        return self.read_ulonglong()

    def read_int8_array(self, n: int) -> Optional[List[int]]:
        return self.read_byte_array(n)

    def read_int16_array(self, n: int) -> Optional[List[int]]:
        return self.read_short_array(n)

    def read_int32_array(self, n: int) -> Optional[List[int]]:
        return self.read_int_array(n)

    def read_int64_array(self, n: int) -> Optional[List[int]]:
        return self.read_longlong_array(n)

    def read_uint8_array(self, n: int) -> Optional[List[int]]:
        return self.read_ubyte_array(n)

    def read_uint16_array(self, n: int) -> Optional[List[int]]:
        return self.read_ushort_array(n)

    def read_uint32_array(self, n: int) -> Optional[List[int]]:
        return self.read_uint_array(n)

    def read_uint64_array(self, n: int) -> Optional[List[int]]:
        return self.read_ulonglong_array(n)


class BinaryWriter(_BaseBinaryWrapper):
    # Write types

    def write_struct(self, fmt: AnyStr, *values):
        self.write(struct.pack("<" + fmt, *values))

    def write_char(self, x: AnyStr):
        self.write_struct("c", x)

    def write_bool(self, x: bool):
        self.write_struct("?", x)

    def write_byte(self, x: int):
        self.write_struct("b", x)

    def write_ubyte(self, x: int):
        self.write_struct("B", x)

    def write_short(self, x: int):
        self.write_struct("h", x)

    def write_ushort(self, x: int):
        self.write_struct("H", x)

    def write_int(self, x: int):
        self.write_struct("i", x)

    def write_uint(self, x: int):
        self.write_struct("I", x)

    def write_long(self, x: int):
        self.write_struct("l", x)

    def write_ulong(self, x: int):
        self.write_struct("L", x)

    def write_longlong(self, x: int):
        self.write_struct("q", x)

    def write_ulonglong(self, x: int):
        self.write_struct("Q", x)

    def write_float(self, x: float):
        self.write_struct("f", x)

    def write_double(self, x: float):
        self.write_struct("d", x)

    def write_string(self, string: AnyStr, size: Optional[int] = None,
                     encoding: Optional[str] = "shift_jis", pad: Optional[bytes] = b"\0"):
        if encoding and isinstance(string, str):
            string = string.encode(encoding)
        if size:
            self.write(string[:size])
            if len(string) < size:
                self.write(pad * (size - len(string)))
        else:
            if len(string) > 0:
                if string[-1] != pad:
                    string += pad
            else:
                string += pad
            self.write(string)

    def write_int24(self, x: int):
        self.write(struct.pack("i", x)[:3])

    def write_uint24(self, x: int):
        self.write(struct.pack("I", x)[:3])

    def write_zeros(self, n: int):
        self.write(b"\0" * n)

    # Arrays
    def write_char_array(self, array: List[AnyStr]):
        self.write_struct(f"{len(array)}c", *array)

    def write_bool_array(self, array: List[bool]):
        self.write_struct(f"{len(array)}c", *array)

    def write_byte_array(self, array: List[int]):
        self.write_struct(f"{len(array)}b", *array)

    def write_ubyte_array(self, array: List[int]):
        self.write_struct(f"{len(array)}B", *array)

    def write_short_array(self, array: List[int]):
        self.write_struct(f"{len(array)}h", *array)

    def write_ushort_array(self, array: List[int]):
        self.write_struct(f"{len(array)}H", *array)

    def write_int_array(self, array: List[int]):
        self.write_struct(f"{len(array)}i", *array)

    def write_uint_array(self, array: List[int]):
        self.write_struct(f"{len(array)}I", *array)

    def write_long_array(self, array: List[int]):
        self.write_struct(f"{len(array)}l", *array)

    def write_ulong_array(self, array: List[int]):
        self.write_struct(f"{len(array)}L", *array)

    def write_longlong_array(self, array: List[int]):
        self.write_struct(f"{len(array)}q", *array)

    def write_ulonglong_array(self, array: List[int]):
        self.write_struct(f"{len(array)}Q", *array)

    def write_float_array(self, array: List[float]):
        self.write_struct(f"{len(array)}f", *array)

    def write_double_array(self, array: List[float]):
        self.write_struct(f"{len(array)}d", *array)

    def write_string_array(self, array: List[AnyStr], size: Optional[int] = None,
                           encoding: Optional[str] = "shift_jis", pad: Optional[str] = b"\0"):
        for string in array:
            self.write_string(string, size, encoding, pad)

    def write_int24_array(self, array: List[int]):
        for x in array:
            self.write_int24(x)

    def write_uint24_array(self, array: List[int]):
        for x in array:
            self.write_uint24(x)

    # Aliasses
    def write_int8(self, x: int):
        self.write_byte(x)

    def write_int16(self, x: int):
        self.write_short(x)

    def write_int32(self, x: int):
        self.write_int(x)

    def write_int64(self, x: int):
        self.write_longlong(x)

    def write_uint8(self, x: int):
        self.write_ubyte(x)

    def write_uint16(self, x: int):
        self.write_ushort(x)

    def write_uint32(self, x: int):
        self.write_uint(x)

    def write_uint64(self, x: int):
        self.write_ulonglong(x)

    def write_int8_array(self, array: List[int]):
        self.write_byte_array(array)

    def write_int16_array(self, array: List[int]):
        self.write_short_array(array)

    def write_int32_array(self, array: List[int]):
        self.write_int_array(array)

    def write_int64_array(self, array: List[int]):
        self.write_longlong_array(array)

    def write_uint8_array(self, array: List[int]):
        self.write_ubyte_array(array)

    def write_uint16_array(self, array: List[int]):
        self.write_ushort_array(array)

    def write_uint32_array(self, array: List[int]):
        self.write_uint_array(array)

    def write_uint64_array(self, array: List[int]):
        self.write_ulonglong_array(array)


class BinaryEditor(BinaryReader, BinaryWriter):
    pass
