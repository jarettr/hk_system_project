/**
 * @file dispatcher.c
 * @brief Secure Runtime Monitor and Code Orchestration Kernel.
 * * Responsible for verifying polymorphic variants via fTPM attestation
 * and performing secure runtime memory injection (Hot-Swap).
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <camkes.h>

/* --- System State --- */
static int total_mutations_count = 0;
static int current_active_variant = 0;

/* --- External Memory Regions (Dataports) --- */
extern void *guest_ram;    // Source: Generator output
extern void *worker_arena; // Destination: Worker RX region
extern void *repo_storage; // Source: Repository metadata

/**
 * @brief Performs Jenkins One-at-a-Time hash for local integrity check.
 */
static uint32_t compute_integrity_hash(const uint8_t *data, size_t len) {
    uint32_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        hash += data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/**
 * @brief Validates code variant and performs secure memory injection.
 * @param variant_id The ID of the variant expected in shared memory.
 */
void execute_secure_runtime_injection(int variant_id) {
    printf("[DISPATCHER] INFO: Validating polymorphic variant %d...\n", variant_id);
    
    /* 1. Extract metadata from Secure Repository */
    uint32_t *metadata = (uint32_t *)repo_storage;
    int stored_vid = (int)metadata[0];
    int payload_size = (int)metadata[1];
    uint32_t expected_signature = metadata[2];

    if (stored_vid != variant_id) {
        printf("[DISPATCHER] ERROR: Variant ID mismatch in repository (Expected: %d, Found: %d).\n", 
                variant_id, stored_vid);
        return;
    }

    /* 2. Compute local hash of the code currently in transit (guest_ram) */
    uint32_t local_hash = compute_integrity_hash((uint8_t *)guest_ram, payload_size);
    printf("[DISPATCHER] DEBUG: Local buffer hash computed: 0x%08X\n", local_hash);

    /* 3. Perform Cryptographic Attestation via fTPM component */
    int is_verified = tpm_verify_api_verify_signature(local_hash, expected_signature);
    
    if (is_verified) {
        /* 4. Integrity confirmed: Perform Secure Code Swap */
        // We copy the verified variant into the Worker's executable region
        memcpy(worker_arena, guest_ram, 4096);
        
        current_active_variant = variant_id;
        total_mutations_count++;
        
        printf("[DISPATCHER] SUCCESS: Integrity verified by fTPM. Code variant %d injected.\n", variant_id);
    } else {
        /* 5. Security Violation detected */
        printf("[DISPATCHER] SECURITY CRITICAL: Attestation FAILED. Variant %d blocked.\n", variant_id);
    }
}

/**
 * @brief Administrative API: Manually trigger a full mutation transaction.
 */
void admin_api_force_mutation(void) {
    int next_id = total_mutations_count + 1;
    
    printf("\n[MONITOR] --- INITIATING SECURE TRANSACTION CYCLE %d ---\n", next_id);
    
    /* Step A: Request new code synthesis from Generator */
    gen_api_request_generation((uint64_t)0x1000, next_id, 5);
    
    /* Step B: Verify attestation and inject into Worker memory */
    execute_secure_runtime_injection(next_id);
    
    /* Step C: Signal Worker to execute the new logic */
    printf("[DISPATCHER] INFO: Requesting execution from Worker sandbox...\n");
    int worker_status = worker_ctrl_cmd(1, 0); 
    
    printf("[DISPATCHER] INFO: Worker execution response: 0x%X\n", worker_status);
    printf("[MONITOR] --- TRANSACTION CYCLE COMPLETE ---\n\n");
}

/**
 * @brief Administrative API: Provide system-wide security telemetry.
 */
void admin_api_print_status(void) {
    printf("\n--- HEURISTIC OS SECURITY TELEMETRY ---\n");
    printf("  Active Code Variant ID:  %d\n", current_active_variant);
    printf("  Total Secure Swaps:      %d\n", total_mutations_count);
    printf("  TPM Attestation Status:  ACTIVE\n");
    printf("---------------------------------------\n");
}

/* seL4 Control Thread Entry Points */
void hw_tick_handle(void) { }
int run(void) { return 0; }
