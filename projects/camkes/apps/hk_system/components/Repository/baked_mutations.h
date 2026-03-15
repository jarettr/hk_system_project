/**
 * @file baked_mutations.h
 * @brief Static pool of pre-verified code mutations.
 * Contains fallback polymorphic payloads for testing and emulation.
 */

#ifndef BAKED_MUTATIONS_H
#define BAKED_MUTATIONS_H

#include <stdint.h>
#include <stddef.h>

/* Emulation pool size for pre-compiled variants */
#define POOL_SIZE 3

/* Variant 0: Reference implementation (NOP sled + RET) */
static const uint8_t variant_0_data[] = { 0x90, 0x90, 0x90, 0xC3 }; 

/* Variant 1: Mutation A (XOR RAX, RAX + RET) */
static const uint8_t variant_1_data[] = { 0x48, 0x31, 0xC0, 0xC3 }; 

/* Variant 2: Mutation B (XOR EAX, EAX + NOP + RET) */
static const uint8_t variant_2_data[] = { 0x31, 0xC0, 0x90, 0xC3 }; 

typedef struct {
    int id;
    const uint8_t *data;
    size_t size;
    const char *hash_str;
} variant_record_t;

static const variant_record_t mutation_pool[POOL_SIZE] = {
    {0, variant_0_data, sizeof(variant_0_data), "hash_00000000"},
    {1, variant_1_data, sizeof(variant_1_data), "hash_11111111"},
    {2, variant_2_data, sizeof(variant_2_data), "hash_22222222"},
};

#endif /* BAKED_MUTATIONS_H */
