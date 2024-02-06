// implement:
//  void uart_init(void)
//
//  int uart_can_get8(void);
//  int uart_get8(void);
//
//  int uart_can_put8(void);
//  void uart_put8(uint8_t c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"


enum {
    AUX_ENB         = 0x20215004,
    AUX_MU_IIR_REG  = 0x20215048,
    AUX_MU_CNTL_REG = 0x20215060,
    AUX_MU_IO_REG   = 0x20215040,
    AUX_MU_IER_REG  = 0x20215044,
    AUX_MU_BAUD_REG = 0x20215068,
    AUX_MU_LCR_REG  = 0x2021504C,
    AUX_MU_LSR_REG  = 0x20215054,
    RX_PIN          = 15,
    TX_PIN          = 14,
    DEFAULT_BAUD    = 271,
};


enum {
    BAUDRATE       = 115200,
    SYS_CLOCK_FREQ = 250 * 1000 * 1000,
    N_DATA_BITS    = 8,
};

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    dev_barrier();

    gpio_set_function(TX_PIN, GPIO_FUNC_ALT5);
    gpio_set_function(RX_PIN, GPIO_FUNC_ALT5);

    dev_barrier();
    // turn on the UART using the AUX_ENB register 
    // 'The AUXENB register is used to enable the three modules; UART, SPI1, SPI2'
    uint32_t enable_val = GET32(AUX_ENB);
    enable_val |= 0x1;
    PUT32(AUX_ENB, enable_val);
    dev_barrier();

    // disable tx/rx so that UART does not process garbage
    uint32_t control_val = GET32(AUX_MU_CNTL_REG);
    control_val &= ~0b11;
    PUT32(AUX_MU_CNTL_REG, control_val);

    // clear the FIFOs
    uint32_t fifo_val = GET32(AUX_MU_IIR_REG);
    fifo_val |= 0b110;
    PUT32(AUX_MU_IIR_REG, 0b110);

    // disable interrupts
    uint32_t ier_val = GET32(AUX_MU_IER_REG);
    ier_val &= ~0b11;
    PUT32(AUX_MU_IER_REG, ier_val);

    PUT32(AUX_MU_BAUD_REG, DEFAULT_BAUD);

    // set the UART to work in 8-bit mode
    uint32_t ld_format = GET32(AUX_MU_LCR_REG);
    ld_format |= 0b11;
    PUT32(AUX_MU_LCR_REG, ld_format);

    // set the baudrate
    // uint32_t baudrate_reg_val = (SYS_CLOCK_FREQ * N_DATA_BITS / BAUDRATE) - 1;
    // PUT32(AUX_MU_BAUD_REG, DEFAULT_BAUD);

    // enable tx/rx so that UART can start working
    control_val = GET32(AUX_MU_CNTL_REG);
    control_val |= 0b11;
    PUT32(AUX_MU_CNTL_REG, control_val); 

    dev_barrier();
}

// disable the uart.
void uart_disable(void) {
    uart_flush_tx();
    dev_barrier();
    uint32_t enable_val = GET32(AUX_ENB);
    enable_val &= ~0x1;
    PUT32(AUX_ENB, enable_val);
    dev_barrier(); 
}


// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_get8(void) {
    // check if data is ready
    while (!uart_has_data()) {}
    // we can read a byte from the rx queue
    return GET32(AUX_MU_IO_REG) & 0xFF;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_put8(void) {
    return (GET32(AUX_MU_LSR_REG) & (1 << 5))? 1 : 0;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
// returns < 0 on error.
int uart_put8(uint8_t c) {
    // check if tx queue can accept 
    while (!uart_can_put8());
    // transmit
    PUT32(AUX_MU_IO_REG, c);
    return 0;
}

// simple wrapper routines useful later.

// 1 = at least one byte on rx queue, 0 otherwise
int uart_has_data(void) {
    return GET32(AUX_MU_LSR_REG) & 0x1;
}

// return -1 if no data, otherwise the byte.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    return (GET32(AUX_MU_LSR_REG) & (1 << 6)) ? 1 : 0;
}

// flush out all bytes in the uart --- we use this when 
// turning it off / on, etc.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
