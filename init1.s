section .init1

extern tty_write_string
global init1
init1:
    push msg
    call tty_write_string
loop:
    jmp loop

msg:
    db "[init1]", 0
