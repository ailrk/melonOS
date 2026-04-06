#pragma once
#include <stdint.h>


/* Layout of the trap frame built on the stack by the
 * hardware and by trapasm.S, and passed to trap().
 *
 * TrapFrame saves the state of the user mode program
 * when it enters the kernel.
 */
typedef struct TrapFrame {
  /* pushed by `trapgo` in `trapasm.s` */
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t _esp; // useless & ingored
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint16_t gs;
  uint16_t _padding1;
  uint16_t fs;
  uint16_t _padding2;
  uint16_t es;
  uint16_t _padding3;
  uint16_t ds;
  uint16_t _padding4;

  /* Pushed by vector.s */
  uint32_t trapno;

  /* Pushed by x86 hardware on interrupt */
  uint32_t err;
  uint32_t eip;
  uint16_t cs;
  uint16_t _padding5;
  uint32_t eflags;

  // Exists when crossing rings, such as from user to kernel
  uint32_t esp;
  uint16_t ss;
  uint16_t _padding6;
} __attribute__((packed)) TrapFrame;


void trap_init();
void trap(TrapFrame *);

#if DEBUG
void dump_trapframe(const TrapFrame *tf);
#endif
