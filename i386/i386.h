#pragma once
// x86 specific instructions

#include <stdint.h>

typedef uint32_t physical_addr;

/* CR0 register */
#define CR0_PE  0x00000001 // 1 = Protected Mode
#define CR0_WP  0x00010000 // write protect
#define CR0_PG  0x80000000 // 1 = Paging, enable paging and use the CR3 register.

/* CR4 register */
#define CR4_PSE 0x00000010  // page size extension
#define CR4_PGE	0x00000080  // page Global Enabled

/* eflags */
#define FL_IF   0x00000200  // Interrupt Enable


static inline unsigned char inb(unsigned short port) {
    unsigned char data;

    asm volatile("in %1, %0" : "=a"(data) : "d"(port));
    return data;
}

#define cli() __asm__ volatile("cli")

#define sti() __asm__ volatile("sti")

#define int_(interrupt) __asm__ volatile("int %0" : : "i" (interrupt))

static inline uint32_t xchg(volatile uint32_t *addr, uint32_t newval) {
  uint32_t result;
  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1"
                : "+m" (*addr), "=a" (result)
                : "1" (newval)
                : "cc");
  return result;
}

static inline void lidt(void *addr) {
    // m should be r and it needs to be derefenced for whatever reason.
    // __asm__ volatile("lidt %0":: "m"(addr));
    __asm__ volatile("lidt (%0)":: "r"(addr));
}


static inline void lgdt(void *addr) {
    __asm__ volatile("lgdt (%0)":: "r"(addr));
}


static inline void ltr(uint16_t sel) {
  asm volatile("ltr %0" : : "r" (sel));
}

static inline void insl(int port, void *addr, int cnt) {
  __asm__ volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void outw(unsigned short port, uint16_t data) {
    __asm__ volatile("out %0, %1" :: "a"(data), "d"(port));
}

static inline void outsl(int port, void const *addr, int cnt) {
    __asm__ volatile("cld; rep outsl"
                     : "=S"(addr), "=c"(cnt)
                     : "d"(port), "0"(addr), "1"(cnt)
                     : "cc");
}

static inline void stosl(void *addr, int data, int cnt) {
  asm volatile("cld; rep stosl" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}


static inline void stosb(void *addr, int data, int cnt) {
    __asm__ volatile("cld; rep stosb"
                     : "=D"(addr), "=c"(cnt)
                     : "0"(addr), "1"(cnt), "a"(data)
                     : "memory", "cc");
}


#define iret() __asm__ volatile ("iret");


/* Wait for a small amount of time by outputting data to an unused port 0x80 */
static inline void io_wait() { outb(0x80, 0); }


/* Switch the current page table entry */
static inline void set_cr3(physical_addr page_dir) { __asm__ volatile("movl %0, %%cr3" : : "r"(page_dir)); }


static inline uint32_t get_cr0() {
  uint32_t v;
  __asm__ volatile("mov %%cr0, %0" : "=r"(v));
  return v;
}

static inline void set_cr0(uint32_t cr0) {
  __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}


static inline uint32_t get_cr4() {
  uint32_t v;
  __asm__ volatile("mov %%cr4, %0" : "=r"(v));
  return v;
}

static inline void set_cr4(uint32_t cr4) {
  __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}


/* Request for the cpu id */
static inline void cpuid(int code, uint32_t* a, uint32_t* d)
{
    __asm__ volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
}


/* read current cpu timestamp */
static inline uint64_t rdtsc() {
    uint64_t ret;
    __asm__ volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}


static inline unsigned readeflags(void) {
  unsigned eflags;
  __asm__ volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}


/* save EFLAGS */
static inline void pushfd() { __asm__ volatile ("pushfd"); }

/* restore EFLAGS */
static inline void popfd() { __asm__ volatile ("popfd"); }


/* write a value to edi for debugging purpose */
static inline void debug_efi(uint32_t val) {
    __asm__ volatile("movl %0,  %%edi\n" : "=r"(val): );
}

