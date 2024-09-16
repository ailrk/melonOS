; Context swtch
;
;   void swtch(struct context **save, struct context *load);
;
; Save the current registers on the stack, creating
; a struct context, and save its address in *old.
; Switch stacks to new and pop previously-saved registers.


global swtch
swtch
    mov eax, [esp + 4] ; save current context to
    mov edx, [esp + 8] ; load new context from

    ; save old calee saved registers
    ; this build's a Context on the stack.
    push ebp
    push ebx
    push esi
    push edi

    mov [eax], esp ; save current context
    mov esp, edx   ; load new context

    ; load new callee saved registers
    ; The context on the stack looks like the following:
    ;  edi;
    ;  esi;
    ;  ebx;
    ;  ebp;
    ;  eip;
    ; We switch to the new process by leaving eip on the
    ; stack then calling ret

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret
