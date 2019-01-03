
// Provided headers
#include "rtc.h"
#include "lib.h"
#include "i8259.h"
/*
void test_interrupts();
*/

void init_rtc(){
	int rate;

	cli();
	outb(REG_B, SELECT_REG);      // select register B, and disable NMI
	char prev = inb(CMOS_RTC_PORT); // read the current value of register B
	outb(REG_B, SELECT_REG);      // set the index again (a read will reset the index to register D)
	outb(prev | 0x40, CMOS_RTC_PORT);

	rate &= 0x0F; //set rate to 2 Hz
	outb(REG_A, SELECT_REG);  //write to REG_A
	prev = inb(CMOS_RTC_PORT);
	outb(REG_A,SELECT_REG); //write to REG_A
	outb((prev & 0xF0)|rate, CMOS_RTC_PORT);

	outb(REG_C, SELECT_REG); //write to REG_C
	inb(CMOS_RTC_PORT);

	//enable IRQs 2 and 8
	enable_irq(rtc_irq);
	enable_irq(rtc_irq2);


	return;
}

void rtc_handler(){
	cli();
    outb(REG_C, SELECT_REG);
    inb(CMOS_RTC_PORT);
    rtc_interrupt_occurred = 1;
		//printf("3");
    /* Signal interrup processing completion by sending EOI */
	sti();
	send_eoi(rtc_irq);

}

//reset frequency to 2 Hz
int32_t rtc_open (const uint8_t* filename) {
    set_rtc_freq(2);
    return 0;
}

//do nothing, return 0
int32_t rtc_close(int32_t fd)
{
    return 0;
}

//return 0 on success, idle if rtc_interrupt_occured not set
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    rtc_interrupt_occurred = 0;

    while(!rtc_interrupt_occurred) {/* stay here */}
		rtc_interrupt_occurred = 0;
    // When rtc_interrupt_occured changes to 1
    return 0;
}

//change frequency according to parameters in fd
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes) {
    //int setterTest = 4;

    if (set_rtc_freq(*(uint8_t*)(buf))) {
        //printf("1"); //single quotes to double quotes?
        return fd;
    }

    return -1;
}

int32_t set_rtc_freq(int32_t freq) {
    char changed_rate;

    switch(freq) {
    case 2:
			changed_rate = HZ_2;
			break;
		case 4:
			changed_rate = HZ_4;
			break;
		case 8:
			changed_rate = HZ_8;
			break;
		case 16:
			changed_rate = HZ_16;
			break;
		case 32:
			changed_rate = HZ_32;
			break;
		case 64:
			changed_rate = HZ_64;
			break;
		case 128:
			changed_rate = HZ_128;
			break;
		case 256:
			changed_rate = HZ_256;
			break;
		case 512:
			changed_rate = HZ_512;
			break;
		case 1024:
			changed_rate = HZ_1024;
			break;
		default:
			return -1;
			break;
    }

    // From OSDev
    cli();
	outb(REG_A, SELECT_REG);        // disable NMI and set index to RTC register A
	char prev = inb(CMOS_RTC_PORT); // get current value of RTC register A
	outb(REG_A, SELECT_REG);        // set index back to RTC reg A
    int32_t set_rate = (prev & 0xF0) | changed_rate;
	outb(set_rate, CMOS_RTC_PORT); // Set the rate
	sti();
	return 1;

}
