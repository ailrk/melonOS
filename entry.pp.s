global _start
; _start is the conventinal elf entry point
; we haven't enable paging yet, but the linker is using the
; virtual address. We need to manually map it to the physical
; address
_start:
extern kmain
extern kstack
extern page_dir
cli
hlt
; push msg1
; call output
; enable page size extension
mov eax, cr4
or eax, (1 << 4)
mov cr4, eax
; push msg2
; call output
; setup cr3 to page directory address.
mov eax, ((page_dir) - 0x80000000)
mov cr3, eax
; push msg2
; call output
; enable paging
mov eax, cr0
or eax, ((1 << 16) | (1 << 31))
mov cr0, eax
; from this point paging is enabled.
mov esp, (kstack + 16384)
mov eax, kmain
jmp eax
; output:
; push ebp
; mov ebp, esp
; mov esi, [esp + 8] ; address of the message
; mov ebx, 0xb8000 ; VGA buffer starts at 0xb800
; output_loop:
; lodsb ; read next byte
; cmp al, 0 ; if it's null
; je output_end ; protect
; or eax, 0x0100 ; set bit 0x0100
; mov word [ebx], ax ;
; add ebx, 2
; jmp output_loop
; output_end:
; pop ebp
; ret
; msg1:
; db "enable PSE", 0
; msg2:
; db "set CR3 to PD", 0
; msg3:
; db "enable paging", 0
