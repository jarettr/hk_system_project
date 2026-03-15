#ifndef BAKED_MUTATIONS_H
#define BAKED_MUTATIONS_H

#include <stdint.h>

// Эмуляция пула из 3-х проверенных статических вариантов
#define POOL_SIZE 3

// Вариант 0 (Эталон)
static const uint8_t variant_0_data[] = { 0x90, 0x90, 0x90, 0xC3 }; // NOP, NOP, NOP, RET
// Вариант 1 (Мутация A)
static const uint8_t variant_1_data[] = { 0x48, 0x31, 0xC0, 0xC3 }; // XOR RAX, RAX, RET
// Вариант 2 (Мутация B)
static const uint8_t variant_2_data[] = { 0x31, 0xC0, 0x90, 0xC3 }; // XOR EAX, EAX, NOP, RET

typedef struct {
    int id;
    const uint8_t* data;
    int size;
    const char* hash;
} variant_record_t;

static const variant_record_t mutation_pool[POOL_SIZE] = {
    {0, variant_0_data, sizeof(variant_0_data), "hash_00000000"},
    {1, variant_1_data, sizeof(variant_1_data), "hash_11111111"},
    {2, variant_2_data, sizeof(variant_2_data), "hash_22222222"},
};

#endif // BAKED_MUTATIONS_H