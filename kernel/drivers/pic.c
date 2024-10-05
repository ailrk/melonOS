#include "i386.h"
#include "drivers/vga.h"
#include "traps.h"
#include <stdint.h>

/* handles hardware interrupts and map them to the system interrupt.
 *
 * PIC has two chips - a master chip and a slave chip. Together they support
 * 15 hardware IRQs. These IRQs are mapped to the CPU interrupts.
 *
 * PIC has two types of command words:
 * - ICW (initialization command word)
 * - OCW (operation command word)
 * */


#define PIC1            0x20        // master pic port
#define PIC2            0xA0        // slave pic port
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)

#define ICW1_ICW4       0x01        // ICW4 will be present
#define ICW1_SINGLE     0x02        // single mode
#define ICW1_INTERVAL4  0x04        // call address interval 4
#define ICW1_LEVEL      0x08        // edge trigged mode
#define ICW1_INIT       0x10        // initialiation

#define ICW4_8086       0x01        // 8086 mod
#define ICW4_AUTO       0x02        // auto EOI
#define ICW4_BUF_SLAVE  0x08        // slave buffered mode
#define ICW4_BUF_MASTER 0x0C        // master buffered mode
#define ICW4_SFNM       0x10        // special fully nested

#define PIC_EOI         0x20


/* send a EOI to pic to indicate end of interrupt handler.
 *
 * If IRQ comes from slave, master needs to be notified too. We simply
 * send to both all the time.
 * */
void pic_eoi() {
    outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}


/* Define the vector offset for pic.
 *
 * In x86 protected mode IRQ 0 to 7 conflicts with the CPU exceptions, so we
 * need to remap the vector to somewhere unused by the CPU. Common way of
 * handling it is to map IRQ0 to vector 0x20.
 *
 * pic follows a specific initialization sequence, at each step data sent from
 * the data port will be interpert as different ICW.
 * */
void pic_remap(uint32_t pic1_offset, uint32_t pic2_offset) {
    uint8_t p1 = inb(PIC1_DATA);                    // save masks
    uint8_t p2 = inb(PIC2_DATA);

    {
        outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // ICW1 starts the initialization
        io_wait();
        outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
        io_wait();
    }

    {
        outb(PIC1_DATA, pic1_offset);               // ICW2 master offset
        io_wait();
        outb(PIC2_DATA, pic2_offset);               // ICW2 slave offset
        io_wait();
    }

    // salve IRQs will be cascade through IRQ2
    {
        outb(PIC1_DATA, 4);                         // ICW3 inform master that slave is on IRQ2
        io_wait();
        outb(PIC2_DATA, 2);                         // ICW3 inform slave how to cascades
        io_wait();
    }

    {
        outb(PIC1_DATA, ICW4_8086);                 // ICW4 use 8086 mode
        io_wait();
        outb(PIC2_DATA, ICW4_8086);                 // ICW4 use 8086 mode
        io_wait();
    }

    outb(PIC1_DATA, p1);                            // restore saved masks
    outb(PIC1_DATA, p2);
}


/* disable pic.
 * In order to use apic we have to disable pic first.
 * */
void pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}



/* masking irq
 *
 * each pic chip has an 8 bit mask register (IMR), 1 bit for each irq indicating
 * whether it should ignoring it. A masked irq will not be raised.
 *
 * normally reading from the data port returns the IMR register. Writing to it
 * set the IMR register.
 * */
void pic_irq_mask(uint8_t irq_line) {
    uint8_t port;
    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    outb(port, inb(port) | (1 << irq_line));
}


void pic_irq_unmask(uint8_t irq_line) {
    uint8_t port;
    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    outb(port, inb(port) & ~(1 << irq_line));
}


void pic_init() {
    vga_printf("[\033[32mboot\033[0m] pic...");
    pic_remap(0x20, 0x20 + 8);
    pic_irq_unmask(I_IRQ_TIMER);
    pic_irq_unmask(I_IRQ_KBD);
    pic_irq_unmask(I_IRQ_CASCADE);
    pic_irq_unmask(I_IRQ_MOUSE);
    pic_irq_unmask(I_IRQ_IDE);
    pic_irq_unmask(I_IRQ_ERR);
    pic_irq_unmask(I_IRQ_SPURIOUS);
    vga_printf("\033[32mok\033[0m\n");
}
