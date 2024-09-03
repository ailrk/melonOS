section .boot1
    bits 16                     ; x86 real mode is 16 bit
    ; org 0x7c00                ; the bootloader start from 0x7c00.
    global boot1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; boot : () -> !
    ; boot into VGA text mode
boot1:

    mov ax, 0x2401
    int 0x15                    ; enable A20 bit

    mov ax, 0x3
    int 0x10                    ; set VGA text mode to 0x3

    call fetch_disk             ; fetch the next sector

    cli                         ; clean interrupt.
                                ; enable 32 bit instructions

    lgdt [gdtr]                 ; load gdt table

    call enter_protected_mode
    call set_segment_registers

    jmp CODE_SEG:boot2          ; long jump to code segment

fetch_disk:
    ; BIOS only loads the first 512 bytes of the
    ; boot sector. We need to mannually load more
    ; memory.
    mov [disk], dl
    mov ah, 0x2                 ; read sectors
    mov al, 36                  ; sector to read
    mov ch, 0                   ; cylinder index
    mov dh, 0                   ; head index
    mov cl, 2                   ; sector index
    mov dl, [disk]              ; disk index
    mov bx, next_sector         ; target pointer
    int 0x13                    ; call disk bios interrupt
    ret

enter_protected_mode:
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    ret

set_segment_registers:
    mov ax, DATA_SEG            ; set seg reg points to data segment.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 16 bits gdt definition 
;; we need at least have null descriptor, code seg and data seg.
;; once the kernel is loaded and virtual memory is setup we need to
;; create a new gdt to use the full range of 32 bits memory.
gdt_null:                       ; setup gdt table.
    dq 0x0                      ; - null descriptor
gdt_code:                       ; code segment (CS)
    dw 0xffff                   ; - segment_limit
    dw 0x0                      ; - base 0 - 15 bits
    db 0x0                      ; - base 16 - 23
    db 0x9a                     ; - access byte
    db 1101111b                 ; - flags 4 bit, lo 4 bit
    db 0x0                      ; - base 24 - 32 bits
gdt_data:                       ; data segment (DS)
    dw 0xffff                   ; the same
    dw 0x0
    db 0x0
    db 0x92
    db 1101111b
    db 0x0
gdt_end:
gdtr:
    dw gdt_end - gdt_null       ; limit
    dd gdt_null                 ; base
disk:
    db 0x0
CODE_SEG equ gdt_code - gdt_null
DATA_SEG equ gdt_data - gdt_null


times 510 - ($-$$) db 0             ; pad til 510 bytes
dw 0xaa55                           ; magic word 0x55AA, little endian for x86.
                                    ; this indicates the end of the 512 bytes sector

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; The second 512 bytes sector
next_sector:
    ; entering 32 bit mode.
    ; in protected mode we can't use bios anymore
    bits 32

    ; boot2 : () -> ()
boot2:
    ; setup an initial C stack for C++ code.
    mov esp, boot_stack_top
    extern boot3
    call boot3
    cli
    hlt

;; end of the second 512 bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

section .bss
align 4

boot_stack_bottom: equ $
        resb 16384                 ; 16kb
boot_stack_top:
