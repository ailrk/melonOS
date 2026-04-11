from ctypes import c_char_p, c_int, c_long, c_size_t, c_void_p, byref, CDLL, POINTER, CFUNCTYPE
import ctypes
import os
import pytest

# Load the shared library
lib_path = os.path.abspath("./test/libutils.so")
lib = CDLL(lib_path)

# ------------------------------
# stblib.c

# atoi: int atoi(char *p)
lib.atoi.argtypes = [c_char_p]
lib.atoi.restype = c_int

# strtol: long int strtol(char *str, char **endptr)
lib.strtol.argtypes = [c_char_p, POINTER(c_char_p)]
lib.strtol.restype = c_long

def test_atoi():
    assert lib.atoi(b"123") == 123
    assert lib.atoi(b"  -42") == -42
    assert lib.atoi(b"0") == 0

def test_strtol_decimal():
    endptr = c_char_p()
    # Testing "1234abc" -> should return 1234, endptr points to 'a'
    res = lib.strtol(b"1234abc", byref(endptr))
    assert res == 1234
    assert endptr.value == b"abc"

def test_strtol_hex():
    res = lib.strtol(b"0x10", None)
    assert res == 16

def test_strtol_octal():
    # Note: Your current code treats leading '0' as octal
    res = lib.strtol(b"010", None)
    assert res == 8

def test_strtol_negative1():
    res = lib.strtol(b"0x0A", None)
    assert res == 10

def test_strtol_negative():
    res = lib.strtol(b"-0x0A", None)
    assert res == -10


# ------------------------------
# string.c


# Define argument/return types for pointer-heavy functions
lib.strlen.argtypes = [c_char_p]
lib.strlen.restype = c_size_t

lib.memset.argtypes = [c_void_p, c_int, c_size_t]
lib.memset.restype = c_void_p

lib.memcpy.argtypes = [c_void_p, c_void_p, c_size_t]
lib.memcpy.restype = c_void_p

lib.strncmp.argtypes = [c_char_p, c_char_p, c_size_t]
lib.strncmp.restype = c_int

lib.strrev.argtypes = [c_char_p]
lib.strrev.restype = c_char_p

lib.strtok.argtypes = [c_char_p, c_char_p]
lib.strtok.restype = c_char_p

# --- Tests ---

def test_strlen():
    assert lib.strlen(b"melonOS") == 7
    assert lib.strlen(b"") == 0

def test_memset():
    buffer = ctypes.create_string_buffer(5)
    lib.memset(buffer, ord('A'), 3)
    # buffer should be b"AAA\x00\x00" (assuming original was nulls)
    assert buffer.value[:3] == b"AAA"

def test_memcpy():
    src = b"Hello"
    dst = ctypes.create_string_buffer(6)
    lib.memcpy(dst, src, 5)
    assert dst.value == b"Hello"

def test_strncmp():
    assert lib.strncmp(b"apple", b"apply", 4) == 0
    assert lib.strncmp(b"apple", b"apply", 5) != 0

def test_strrev():
    # strrev usually modifies string in-place.
    # Must use create_string_buffer because b"" literals are immutable in Python.
    buf = ctypes.create_string_buffer(b"olleh")
    res = lib.strrev(buf)
    assert res == b"hello"

def test_strtok():
    s = ctypes.create_string_buffer(b"apple,orange,banana")
    delim = b","
    assert lib.strtok(s, delim) == b"apple"
    assert lib.strtok(None, delim) == b"orange"
    assert lib.strtok(None, delim) == b"banana"
    assert lib.strtok(None, delim) is None

def test_strchr():
    # char *strchr(const char *s, int chr);
    lib.strchr.restype = ctypes.c_char_p
    s = b"melon"
    assert lib.strchr(s, ord('l')) == b"lon"
    assert lib.strchr(s, ord('z')) is None
