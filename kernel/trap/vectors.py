#!/usr/bin/python3
# sample output:
#   ; handlers
#   extern trapgo
#   global vector0
#   vector0:
#     push 0
#     jmp trapgo
#   ...
#
#   ; vector table
#   global vectors
#   vectors:
#     dd vector0
#     dd vector1
#     dd vector2
#   ...

def p(s: str, indent = 0):
    if indent > 0:
        print("  " * indent, end = "")
    print(s)

def p1(s): p(s, 1)

p("; generated by `vectors.py`, do not edit")
p("; handlers")
p("extern trapgo")
for i in range(0, 256):
    p(f"global vector{i}")
    p(f"vector{i}:")
    # if the interrupt doesn't push and error code we manually
    # insert an 0
    if not ((i == 8 or (i >= 10 and i <= 14)) or i == 17):
        p1(f"push 0x0")
    p1(f"push {i}")
    p1(f"jmp trapgo")


p(" ")
p("; vector table")
p("global vectors:")
p("vectors:")
for i in range(0, 256):
    p1(f"dd vector{i}")
