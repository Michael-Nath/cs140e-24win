// engler, cs140 put your gpio-int implementations in here.
#include "rpi.h"
#include "rpi-interrupts.h"


enum {
    GPIO_INT0_LOC = 17, // location of gpio_int[0] interrupt source relative to IRQS_2
    GPEDS_0 = 0x20200040, // GPIO Event Detect Status
    GPREN_0 = 0x2020004C, // GPIO Rising Edge Enable
    GPFEN_0 = 0x20200058, // GPIO Falling Edge Enable
    GPHEN_0 = 0x20200064, // GPIO High Detect Enable,
    GPLEN_0 = 0x20200070, // GPIO Low Detect Enable,
};

// returns 1 if there is currently a GPIO_INT0 interrupt, 
// 0 otherwise.
//
// note: we can only get interrupts for <GPIO_INT0> since the
// (the other pins are inaccessible for external devices).
int gpio_has_interrupt(void) {
    // check the correct IRQ_pending register
    int gpio_int_0_idx = 17; // gpio_int[0] is index 17 in IRQ_pending_2
    uint32_t val = GET32(IRQ_pending_2) & (1 << gpio_int_0_idx);
    return DEV_VAL32(val > 0);
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    if (pin >= 32)
        return;
    // enable in the GPREN register
    dev_barrier();
    uint32_t val = GET32(GPREN_0);
    val = val | (1 << pin);
    PUT32(GPREN_0, val);
    dev_barrier();
    PUT32(Enable_IRQs_2, 1 << GPIO_INT0_LOC);
    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    if (pin >= 32)
        return;
    dev_barrier();
    uint32_t val = GET32(GPFEN_0);
    val = val | (1 << pin);
    PUT32(GPFEN_0, val);
    dev_barrier();
    PUT32(Enable_IRQs_2, 1 << GPIO_INT0_LOC);
    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    if (pin >= 32)
        return 0;
    dev_barrier();
    uint32_t val = GET32(GPEDS_0) & (1 << pin); 
    dev_barrier();
    return DEV_VAL32(val > 0);
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    if (pin >= 32)
        return;
    dev_barrier();
    PUT32(GPEDS_0, 1 << pin);
    dev_barrier();
}
