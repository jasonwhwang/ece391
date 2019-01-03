#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#define PIT_FREQ 1193180
#define COM_BYTE 0x36
#define PIT_PORT 0x43
#define CHAN_0 0x40
#define LOW_MASK 0xFF
#define EIGHT 8
#define PIT_IRQ 0

void init_pit();
void pit_handler();

#endif
