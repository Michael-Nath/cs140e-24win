// very dumb, simple interface to wrap up watchpoints better.
// only handles a single watchpoint.
//
// You should be able to take most of the code from the 
// <1-watchpt-test.c> test case where you already did 
// all the needed operations.  This interface just packages 
// them up a bit.
//
// possible extensions:
//   - do as many as hardware supports, perhaps a handler for 
//     each one.
//   - make fast.
//   - support multiple independent watchpoints so you'd return
//     some kind of structure and pass it in as a parameter to
//     the routines.
#include "rpi.h"
#include "mini-watch.h"

// we have a single handler: so just use globals.
static watch_handler_t watchpt_handler = 0;
static void *watchpt_data = 0;

// is it a load fault?
static int mini_watch_load_fault(void) {
    return datafault_from_ld();
}

// if we have a dataabort fault, call the watchpoint
// handler.
static void watchpt_fault(regs_t *r) {
    // watchpt handler.
    if(was_brkpt_fault())
        panic("should only get debug faults!\n");
    if(!was_watchpt_fault())
        panic("should only get watchpoint faults!\n");
    if(!watchpt_handler)
        panic("watchpoint fault without a fault handler\n");

    watch_fault_t w = {0};
    w.fault_pc = watchpt_fault_pc();
    uint32_t offset = 0;
    uint32_t byte_switch = bits_get(cp14_wcr0_get(), 5, 8);
    while (byte_switch != 1) {
        offset++;
        byte_switch = byte_switch >> 1;
    }
    w.fault_addr = (void*) cp14_wvr0_get() + offset;
    w.is_load_p = mini_watch_load_fault();
    w.regs = r;
    watchpt_handler(watchpt_data, &w);
    // in case they change the regs.
    switchto(w.regs);
}

// setup:
//   - exception handlers, 
//   - cp14, 
//   - setup the watchpoint handler
// (see: <1-watchpt-test.c>
void mini_watch_init(watch_handler_t h, void *data) {
    // todo("setup cp14 and the full exception routines");

    // install exception handlers: see <staff-full-except.c>
    full_except_install(0);
    full_except_set_data_abort(watchpt_fault);

    cp14_enable();
    // just started, should not be enabled.
    assert(!cp14_bcr0_is_enabled());
    assert(!cp14_bcr0_is_enabled());

    watchpt_handler = h;
    watchpt_data = data;
}

// set a watch-point on <addr>.
void mini_watch_addr(void *addr) {
    uint32_t u_addr = (uint32_t) addr;
    if (!mini_watch_enabled())
        cp14_wcr0_enable();
    cp14_wvr0_set(u_addr);
    uint32_t offset = u_addr - ((u_addr / 4) * 4);
    uint32_t wcr = cp14_wcr0_get();
    wcr = bits_set(wcr, 5, 8, 1 << offset);
    cp14_wcr0_set(wcr);
    assert(cp14_wcr0_is_enabled());
}

// disable current watchpoint <addr>
void mini_watch_disable(void *addr) {
    cp14_wcr0_disable();
}

// return 1 if enabled.
int mini_watch_enabled(void) {
    return cp14_wcr0_is_enabled() && cp14_is_enabled();
}

// called from exception handler: if the current 
// fault is a watchpoint, return 1
int mini_watch_is_fault(void) { 
    return was_watchpt_fault();
}
