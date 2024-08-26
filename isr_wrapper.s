%macro isr_wrapper 1
global isr_%+%1
align  4
isr_%+%1:
    pushad
    cld    ; Some C ABI requires DF to be clear on function entry
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

isr_wrapper I_DIVBYZERO
isr_wrapper I_DEBUG
isr_wrapper I_NMI
isr_wrapper I_BRKPNT
isr_wrapper I_OVERFLOW
isr_wrapper I_BOUND
isr_wrapper I_ILLEGALOP
isr_wrapper I_COPNOAVIL
isr_wrapper I_DOUBLEFLT
isr_wrapper I_COPSEG
isr_wrapper I_TSS
isr_wrapper I_SEGNP
isr_wrapper I_STKSGFLT
isr_wrapper I_GPFLT
isr_wrapper I_PGFLT
isr_wrapper I_FPERR
isr_wrapper I_ALIGN
isr_wrapper I_MACHINE
isr_wrapper I_SIMDERR
isr_wrapper I_IRQ_TIMER
isr_wrapper I_IRQ_KBD
isr_wrapper I_IRQ_COM1
isr_wrapper I_IRQ_IDE
isr_wrapper I_IRQ_ERR
isr_wrapper I_IRQ_SPURIOUS
isr_wrapper I_SYSCALL
isr_wrapper I_DEFAULT
