section .init1

global init1
init1:
loop: jmp loop

msg:
    db "init1", 0
