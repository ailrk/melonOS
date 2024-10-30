#pragma once

#include <stdint.h>

void pic_eoi ();
void pic_remap (uint32_t pic1_offset, uint32_t pic2_offset);
void pic_disable (void);
void pic_irq_mask (uint8_t irq_line);
void pic_irq_unmask (uint8_t irq_line);
void pic_init ();
