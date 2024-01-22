/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34),
    gpio_fsel0 = 0x20200000
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    gpio_set_function(pin, );
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
  // implement this
  // use <gpio_set0>
  PUT32(gpio_set0, 0x1 << pin);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32 && pin != 47)
        return;
  // implement this
  // use <gpio_clr0>
  PUT32(gpio_clr0, 0x1 << pin);
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
  // implement.
  if(pin >= 32)
        return;
  
  unsigned* corresponding_fsel = (unsigned*) gpio_fsel0 + (pin / 10);
  unsigned relative = (pin % 10) * 3;
  unsigned mask = 0b111 << relative;
  unsigned masked_out = get32(corresponding_fsel);
  masked_out &= ~mask;
  put32(corresponding_fsel, masked_out);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
  unsigned v = 0;
  if (pin >= 32)
    return -1;

  // implement.
  unsigned mask = ~(0x1 << pin);
  v = GET32(gpio_lev0);
  v &= ~mask;
  v = v >> pin;
  return DEV_VAL32(v);
}

void gpio_set_function(unsigned pin, gpio_func_t function) {
  if(pin >= 32 && pin != 47)
    return;
  if (function & ~0b111)
    return;
  unsigned* corresponding_fsel = (unsigned*) gpio_fsel0 + (pin / 10);
  unsigned relative = (pin % 10) * 3;
  unsigned mask = 0b111 << relative;
  unsigned masked_out = get32(corresponding_fsel);
  masked_out &= ~mask;
  unsigned new_register_value = masked_out | (function << relative);
  put32(corresponding_fsel, new_register_value); 
}