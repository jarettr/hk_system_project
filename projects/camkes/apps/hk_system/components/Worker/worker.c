/**
 * @file worker.c
 * @brief Isolated Execution Sandbox (Worker Component).
 * Operates in a highly restricted environment (RX memory only for executable_arena).
 * Directly executes polymorphic machine code verified by the Dispatcher.
 */

#include <stdio.h>
#include <stdint.h>
#include <camkes.h>

/* Shared memory region with RX capabilities (set in CAmkES config) */
extern void *executable_arena;

/* =====================================================================
 * VERIFICATION INTERFACE
 * ===================================================================== */

/**
 * @brief Computes the runtime hash of the execution arena.
 * Allows the Verifier to audit memory integrity post-injection.
 */
uint32_t verifier_target_get_hash(void) {
    uint8_t *ram = (uint8_t *)executable_arena;
    uint32_t hash = 0;
    
    /* Jenkins One-at-a-Time Hash */
    for(int i = 0; i < 4096; i++) {
        hash += ram[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    return hash;
}

/* =====================================================================
 * JIT EXECUTION CONTROL
 * ===================================================================== */

/**
 * @brief Handles control commands from the Dispatcher.
 * @param type Instruction type (1 = Execute).
 * @param payload Optional arguments for execution.
 */
int ctrl_api_cmd(int type, int payload) {
    if (type == 1) { 
        printf("[WORKER] INFO: Execution directive received. Transferring control to JIT arena...\n");
        
        /* * Cast the secure Dataport memory region to a callable function pointer.
         * This operates strictly within hardware-enforced NX/DEP bounds.
         */
        uint32_t (*jit_task)(void) = (uint32_t (*)(void))executable_arena;
        
        /* Hardware-native execution of the polymorphic payload */
        uint32_t result = jit_task();
        
        printf("[WORKER] SUCCESS: Native task execution completed. Result: 0x%08X\n", result);
        return (int)result;
    }
    return 0;
}

int run(void) { 
    /* Worker remains in standby, awaiting RPC signals from the Dispatcher */
    return 0; 
}
