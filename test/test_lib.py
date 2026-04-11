from ctypes import c_char_p, c_int, c_long, byref, CDLL, POINTER
import os
import pytest

# Load the shared library
lib_path = os.path.abspath("./test/libutils.so")
lib = CDLL(lib_path)

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

def test_strtol_negative():
    res = lib.strtol(b"-0x0A", None)
    assert res == -10
