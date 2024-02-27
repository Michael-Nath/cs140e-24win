/*
 * simplified i2c implementation --- no dma, no interrupts.  the latter
 * probably should get added.  the pi's we use can only access i2c1
 * so we hardwire everything in.
 *
 * datasheet starts at p28 in the broadcom pdf.
 *
 */
#include "rpi.h"
#include "libc/helper-macros.h"
#include "i2c.h"
#include "libc/bit-support.h"

typedef struct {
	uint32_t control; // "C" register, p29
	uint32_t status;  // "S" register, p31

#	define check_dlen(x) assert(((x) >> 16) == 0)
	uint32_t dlen; 	// p32. number of bytes to xmit, recv
					// reading from dlen when TA=1
					// or DONE=1 returns bytes still
					// to recv/xmit.  
					// reading when TA=0 and DONE=0
					// returns the last DLEN written.
					// can be left over multiple pkts.

    // Today address's should be 7 bits.
#	define check_dev_addr(x) assert(((x) >> 7) == 0)
	uint32_t 	dev_addr;   // "A" register, p 33, device addr 

	uint32_t fifo;  // p33: only use the lower 8 bits.
#	define check_clock_div(x) assert(((x) >> 16) == 0)
	uint32_t clock_div;     // p34
	// we aren't going to use this: fun to mess w/ tho.
	uint32_t clock_delay;   // p34
	uint32_t clock_stretch_timeout;     // broken on pi.
} RPI_i2c_t;

// offsets from table "i2c address map" p 28
_Static_assert(offsetof(RPI_i2c_t, control) == 0, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, status) == 0x4, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, dlen) == 0x8, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, dev_addr) == 0xc, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, fifo) == 0x10, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, clock_div) == 0x14, "wrong offset");
_Static_assert(offsetof(RPI_i2c_t, clock_delay) == 0x18, "wrong offset");

/*
 * There are three BSC masters inside BCM. The register addresses starts from
 *	 BSC0: 0x7E20_5000 (0x20205000)
 *	 BSC1: 0x7E80_4000
 *	 BSC2 : 0x7E80_5000 (0x20805000)
 * the PI can only use BSC1.
 */
static volatile RPI_i2c_t *i2c = (void*)0x20804000; 	// BSC1

// extend so this can fail.
int i2c_write(unsigned addr, uint8_t data[], unsigned nbytes) {
    dev_barrier();
	// wait until there is no active transfer
    while (bit_is_on(i2c->status, 0)){};
	// check fifo is empty
	if (bit_is_on(i2c->control, 6))
		panic("FIFO is not empty!\n");

	// check there is no clock stretch timeout errors
	if (!bit_is_off(i2c->control, 9))
		panic("There is clock stretch timeotu error!\n");

	// set device address and length
	PUT32((uint32_t) &i2c->dlen, nbytes);
	PUT32((uint32_t) &i2c->dev_addr, addr);

	// set control reg to write and start transfer
	uint32_t new_cntrl_val = 0;
	new_cntrl_val = bit_clr(new_cntrl_val, 0); // notify that this will be write
	new_cntrl_val = bit_set(new_cntrl_val, 15);
	new_cntrl_val = bit_set(new_cntrl_val, 7); // start the transfer
	PUT32((uint32_t) &i2c->control, new_cntrl_val);

	// wait until transfer has started and is active
	while (bit_is_off(i2c->status, 0));
	for (unsigned i = 0; i < nbytes; i++) {
		// make sure there is space in the FIFO to write a byte
		while (bit_is_off(i2c->status, 4));
		PUT32((uint32_t) &i2c->fifo, data[i]);
	}
	// wait for transfer to be marked as done
	while (bit_is_off(i2c->status, 1));

	// clear this field
	uint32_t status = i2c->status;
	status = bit_set(status, 1);
	PUT32((uint32_t) &i2c->status, status);
	assert((bit_is_off(i2c->status, 0)));
	dev_barrier();
	return 1;
}

// extend so it returns failure.
int i2c_read(unsigned addr, uint8_t data[], unsigned nbytes) {
	dev_barrier();
	// wait until there is no active transfer
    while (bit_is_on(i2c->status, 0)) {};

	// check fifo is empty
	if (bit_is_on(i2c->control, 6))
		panic("FIFO is not empty!\n");

	// check there is no clock stretch timeout errors
	if (!bit_is_off(i2c->control, 9))
		panic("There is clock stretch timeotu error!\n");

	// set device address and length
	PUT32((uint32_t) &i2c->dlen, nbytes);
	PUT32((uint32_t) &i2c->dev_addr, addr);

	// set control reg to read and start transfer
	uint32_t new_cntrl_val = 0;
	new_cntrl_val = bit_set(new_cntrl_val, 0);
	new_cntrl_val = bit_set(new_cntrl_val, 15);
	new_cntrl_val = bit_set(new_cntrl_val, 7);
	PUT32((uint32_t) &i2c->control, new_cntrl_val);

	// wait until transfer has started and is active
	while (bit_is_off(i2c->status, 0));

	for (unsigned i = 0; i < nbytes; i++) {
		// make sure there is a byte ready to be read
		while (bit_is_off(i2c->status, 5)) {};
		data[i] = i2c->fifo;
	}
	// wait for transfer to be marked as done
	while (bit_is_off(i2c->status, 1));
	uint32_t status = i2c->status;
	status = bit_set(status, 1);
	PUT32((uint32_t) &i2c->status, status);
	
	assert(bit_is_on(i2c->control, 15));
	assert(bit_is_off(i2c->status, 0));
	dev_barrier();
	return 1;
}

void i2c_init(void) {
	dev_barrier();
	gpio_set_function(2, GPIO_FUNC_ALT0);
	gpio_set_function(3, GPIO_FUNC_ALT0);
	dev_barrier();
	// enable the bsc
	uint32_t control_reg = GET32((uint32_t) &i2c->control);
	control_reg = bit_set(control_reg, 15);
	PUT32((uint32_t) &i2c->control, control_reg);
	// set the divider register
	assert((GET32((uint32_t) &i2c->clock_div)) == 0x5dc);
	uint32_t reset_status_value = 1100000010;
	PUT32((uint32_t) &i2c->status, reset_status_value);
	uint32_t status = GET32((uint32_t) &i2c->status);
	if (bit_is_on(status, 0))
		panic("There is an active transfer going on!\n");
	assert(bit_is_on(i2c->control, 15));
	dev_barrier();	
}

// shortest will be 130 for i2c accel.
void i2c_init_clk_div(unsigned clk_div) {
	PUT32((uint32_t) &i2c->clock_div, clk_div);
}
