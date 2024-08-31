%macro isr_wrapper 2
global isr_%+%1
align 4
isr_%+%1:
pushad
cld ; Some C ABI requires DF to be clear on function entry
call handle_%+%1
popad
iret
%endmacro
extern handle_I_DIVBYZERO
extern handle_I_DEBUG
extern handle_I_NMI
extern handle_I_BRKPNT
extern handle_I_OVERFLOW
extern handle_I_BOUND
extern handle_I_ILLEGALOP
extern handle_I_COPNOAVIL
extern handle_I_DOUBLEFLT
extern handle_I_COPSEG
extern handle_I_TSS
extern handle_I_SEGNP
extern handle_I_STKSGFLT
extern handle_I_GPFLT
extern handle_I_PGFLT
extern handle_I_FPERR
extern handle_I_ALIGN
extern handle_I_MACHINE
extern handle_I_SIMDERR
extern handle_I_IRQ_TIMER
extern handle_I_IRQ_KBD
extern handle_I_IRQ_COM1
extern handle_I_IRQ_IDE
extern handle_I_IRQ_ERR
extern handle_I_IRQ_SPURIOUS
extern handle_I_SYSCALL
extern handle_I_DEFAULT
isr_wrapper I_DIVBYZERO, 0
isr_wrapper I_DEBUG, 1
isr_wrapper I_NMI, 2
isr_wrapper I_BRKPNT, 3
isr_wrapper I_OVERFLOW, 4
isr_wrapper I_BOUND, 5
isr_wrapper I_ILLEGALOP, 6
isr_wrapper I_COPNOAVIL, 7
isr_wrapper I_DOUBLEFLT, 8
isr_wrapper I_COPSEG, 9
isr_wrapper I_TSS, 10
isr_wrapper I_SEGNP, 11
isr_wrapper I_STKSGFLT, 12
isr_wrapper I_GPFLT, 13
isr_wrapper I_PGFLT, 14
isr_wrapper I_FPERR, 16
isr_wrapper I_ALIGN, 17
isr_wrapper I_MACHINE, 18
isr_wrapper I_SIMDERR, 19
isr_wrapper I_IRQ_TIMER, 32
isr_wrapper I_IRQ_KBD, (32 + 1)
isr_wrapper I_IRQ_COM1, (32 + 4)
isr_wrapper I_IRQ_IDE, (32 + 14)
isr_wrapper I_IRQ_ERR, (32 + 19)
isr_wrapper I_IRQ_SPURIOUS, (32 + 31)
isr_wrapper I_SYSCALL, 64
isr_wrapper I_DEFAULT, 255
