/**
 * @file admin.c
 * @brief Administrative Control Console for Heuristic OS.
 * Provides a command interface for system telemetry and 
 * manual logic mutation orchestration.
 */

#include <stdio.h>
#include <stdint.h>
#include <camkes.h>

/**
 * @brief Symbolic delay for log readability.
 * Uses volatile to prevent compiler optimization.
 */
static void symbolic_sleep(uint64_t iterations) {
    for (volatile uint64_t i = 0; i < iterations; i++);
}

/**
 * @brief Component entry point.
 */
int run(void) {
    /* 1. Initialization Phase */
    // Wait for the kernel and other components to settle their boot logs
    symbolic_sleep(80000000);

    printf("\n--------------------------------------------\n");
    printf("  HEURISTIC OS - ADMINISTRATIVE CONSOLE     \n");
    printf("  Secure Execution Monitor: ACTIVE          \n");
    printf("--------------------------------------------\n");

    while(1) {
        /* 2. Telemetry Phase */
        printf("\n[ADMIN] INFO: Requesting system security telemetry...\n");
        // Request the current state from the Dispatcher
        admin_api_print_status();

        symbolic_sleep(20000000);

        /* 3. Transaction Phase */
        printf("[ADMIN] ACTION: Dispatching FORCE_MUTATION command to Kernel Dispatcher...\n");
        
        /**
         * Trigger the secure transaction. 
         * This calls the Dispatcher, which then orchestrates:
         * Generator -> Repository -> fTPM -> Worker
         */
        admin_api_force_mutation();
        
        printf("[ADMIN] SUCCESS: Mutation transaction acknowledged.\n");

        /* 4. Cooldown Phase */
        // Long delay between cycles for clear audit logs
        printf("[ADMIN] INFO: Entering standby for 10 seconds...\n");
        symbolic_sleep(800000000);
    }

    return 0;
}
