/**
 * @file scheduler.c
 * @brief Hardware Timer and Scheduling Controller.
 * Generates periodic scheduling ticks (notifications) for the Dispatcher
 * to orchestrate system-wide events and mutations.
 */

#include <stdio.h>
#include <stdint.h>
#include <camkes.h>

/* Default scheduling quantum in milliseconds */
static uint32_t current_quantum_ms = 500;

/**
 * @brief RPC interface to adjust the timer quantum.
 * @param milliseconds The new quantum duration.
 */
void ctrl_set_quantum(int milliseconds) {
    printf("[SCHEDULER] WARNING: Quantum adjusted from %u ms to %d ms via RPC.\n", 
           current_quantum_ms, milliseconds);
    current_quantum_ms = (uint32_t)milliseconds;
}

int run(void) {
    printf("[SCHEDULER] INFO: Hardware timer proxy initialized. Base quantum = %u ms.\n", current_quantum_ms);
    
    while (1) {
        /* * Simulated hardware delay.
         * In a production environment, this interfaces with PIT/HPET or ARM Generic Timer.
         */
        volatile uint64_t delay = (uint64_t)current_quantum_ms * 100000ULL; 
        for(volatile uint64_t i = 0; i < delay; i++); 
        
        /* Emit hardware tick notification to the Dispatcher */
        hw_tick_emit(); 
    }
    
    return 0;
}
