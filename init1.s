section .init1

extern tty_printf
global init1
init1:
    push msg
    call tty_printf
loop:
    jmp loop

msg:
    db "[init1]", 0
