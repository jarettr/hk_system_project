/**
 * @file hk_sdk.h
 * @brief Heuristic OS Software Development Kit.
 * Provides compiler annotations and API endpoints for managing 
 * runtime dynamic polymorphism and secure hot-swapping.
 */

#ifndef HK_SDK_H
#define HK_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================
 * LLVM COMPILER ANNOTATIONS (MUTATION ENGINE)
 * ===================================================================== */

/** * @def HK_MUTATE
 * @brief Enables aggressive code mutation (junk-code insertion, CFG alterations).
 * Parsed by the custom LLVM Mutator Pass during synthesis.
 */
#define HK_MUTATE __attribute__((annotate("hk_mutate")))

/** * @def HK_NO_MUTATE
 * @brief Exempts the function from all mutations. 
 * Mandatory for strict real-time (RT) critical paths and hardware interactions
 * where timing consistency is required.
 */
#define HK_NO_MUTATE __attribute__((annotate("hk_no_mutate")))

/** * @def HK_MUTATE_RO
 * @brief Restricts mutation to data-flow only (variables/constants). 
 * Preserves the original Control Flow Graph (CFG).
 */
#define HK_MUTATE_RO __attribute__((annotate("hk_mutate_ro")))

/* =====================================================================
 * HOT-SWAP STATE LIFECYCLE API
 * ===================================================================== */

/**
 * @brief Function pointer type for component lifecycle callbacks.
 */
typedef void (*hk_callback_t)(void);

/**
 * @brief Registers a callback to be executed immediately BEFORE a code mutation.
 * Used for saving local component state (registers, variables) to secure persistent memory.
 * * @param callback Function pointer to the handler.
 */
void hk_register_pre_mutation_callback(hk_callback_t callback);

/**
 * @brief Registers a callback to be executed immediately AFTER a new variant is injected.
 * Used for restoring component state and resuming operations in the new execution arena.
 * * @param callback Function pointer to the handler.
 */
void hk_register_post_mutation_callback(hk_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* HK_SDK_H */
