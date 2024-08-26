#pragma once 
// x86 specific instructions

#include <stdint.h>
#define DEF_I386 static inline

DEF_I386 unsigned char inb(unsigned short port) {
    unsigned char data;

    asm volatile("in %1, %0" : "=a"(data) : "d"(port));
    return data;
}

DEF_I386 void cli() {
    __asm__ volatile("cli");
}

DEF_I386 void sti() {
    __asm__ volatile("sti");
}

#define int_(interrupt) __asm__ volatile("int %0" : : "i" (interrupt))

DEF_I386 uint32_t xchg(volatile uint32_t *addr, uint32_t newval) {
  uint32_t result;
  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" 
                : "+m" (*addr), "=a" (result) 
                : "1" (newval) 
                : "cc");
  return result;
}

DEF_I386 void lidt(void *addr) {
    // m should be r and it needs to be derefenced for whatever reason.
    // __asm__ volatile("lidt %0":: "m"(addr));
    __asm__ volatile("lidt (%0)":: "r"(addr));
}

DEF_I386 void insl(int port, void *addr, int cnt) {
    __asm__ volatile("cld; rep insl"
                     : "=m"(addr), "=c"(cnt)
                     : "d"(port), "0"(addr), "1"(cnt)
                     : "memory", "cc");
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

DEF_I386 void outw(unsigned short port, uint16_t data) {
    __asm__ volatile("out %0, %1" :: "a"(data), "d"(port));
}

DEF_I386 void outsl(int port, void const *addr, int cnt) {
    __asm__ volatile("cld; rep outsl"
                     : "=S"(addr), "=c"(cnt)
                     : "d"(port), "0"(addr), "1"(cnt)
                     : "cc");
}

DEF_I386 void stosb(void *addr, int data, int cnt) {
    __asm__ volatile("cld; rep stosb"
                     : "=D"(addr), "=c"(cnt)
                     : "0"(addr), "1"(cnt), "a"(data)
                     : "memory", "cc");
}


DEF_I386 void iret() {
    __asm__ volatile ("iret");
}


/* write a value to edi for debugging purpose */
DEF_I386 void debug_efi(uint32_t val) {
    __asm__ volatile("movl %0,  %%edi\n" : "=r"(val): ); 
}
