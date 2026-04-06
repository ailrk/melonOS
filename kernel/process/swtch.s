; Context switching
;
;   void swtch(struct context **save, struct context *load);
;
; Save the current registers on the stack, creating
; a struct context, and save its address in `save`.
;
; Switch stacks to new and pop previously-saved registers.

global swtch
swtch
    ; (Addresses are hypothetical)
    ; When swtch is called, the stack looks like this:
    ;  0x90 eip  <-esp-+
    ;  0x94 load       |
    ;  0x98 save <-----+
    ; $eip is where we return back to.
    mov eax, [esp + 4] ; Where to save current context
    mov edx, [esp + 8] ; Where to load new context from

    ; Save old callee saved registers
    ; this build's a Context on the stack.
    push ebp
    push ebx
    push esi
    push edi
    ; The kernel stack:
    ;  0x80 edi <-esp-+
    ;  0x84 esi       |
    ;  0x88 ebx       | old Context
    ;  0x8c ebp       |
    ;  0x90 eip <-----+
    ;  0x94 load
    ;  0x98 save
    ; We will leave it here. $eip tells us where to go back to.

    mov [eax], esp ; Save current Context on the stack to `save`.

    mov esp, edx   ; load new context.
                   ; Now $esp points to the new stack.
                   ; We are at a different kernel stack.

    ; The stack looks like this:
    ;  0x60 edi <-esp-+
    ;  0x64 esi       |
    ;  0x68 ebx       | new Context
    ;  0x6c ebp       |
    ;  0x70 eip <-----+

    ; Pop registers and only keep $eip
    pop edi
    pop esi
    pop ebx
    pop ebp

    ; $eip is on the stack, ret takes the new eip and jump
    ; to the new process.
    ret
    ; note:
    ; We cannot mov an address to $eip. With `ret`, we take
    ; whatever on the top of the stack and jump there blindly.


; Why ABI expects ebp, ebx, esi, edi to callee saved?
; No perticular reason.
;
; We can make everything caller saved, but that will be inefficient
; in a lot of cases because no matter what we do we have to save
; all registers on calls.
