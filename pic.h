#pragma once

#include <stdint.h>
#include "i386.h"

void pic_send_eoi(uint8_t irq);
void pic_remap(uint32_t pic1_offset, uint32_t pic2_offset);
void pic_disable(void);
void pic_irq_mask(uint8_t irq_line);
void pic_irq_unmask(uint8_t irq_line);
void pic_init();
