/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask, slave_mask;
/* Initialize the 8259 PIC */
void i8259_init(void) {

  //set masks
  master_mask = MASK;
  slave_mask = MASK;

  //send control words for master
  outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_DATA);
	outb(ICW3_MASTER, MASTER_DATA);
	outb(ICW4, MASTER_DATA);

  //send control words for slave
  outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_DATA);
	outb(ICW3_SLAVE, SLAVE_DATA);
	outb(ICW4, SLAVE_DATA);

  //restore masks
  outb(master_mask, MASTER_DATA);
	outb(slave_mask, SLAVE_DATA);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
  uint16_t port;
  if (irq_num < MIDPOINT) {
		port = MASTER_DATA;
    master_mask = master_mask & ~(1 << irq_num);
    outb(master_mask,port);
	}
  else {
		port = SLAVE_DATA;
		irq_num = irq_num - MIDPOINT;
    slave_mask = slave_mask & ~(1 << irq_num);
    outb(slave_mask,port);
	}
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  uint16_t port;
  if (irq_num < MIDPOINT) {
		port = MASTER_DATA;
    master_mask = master_mask | (1 << irq_num);
    outb(master_mask,port);
	}
  else {
		port = SLAVE_DATA;
		irq_num = irq_num - MIDPOINT;
    slave_mask = slave_mask | (1 << irq_num);
    outb(slave_mask,port);
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
  if(irq_num >= MIDPOINT) {
		outb(EOI | MASK_SLAVE_IRQ, MASTER_8259_PORT);
    outb(EOI | (irq_num-MIDPOINT), SLAVE_8259_PORT);
	}
  else
    outb((EOI | irq_num), MASTER_8259_PORT);
}
