/**
 * @file generator.c
 * @brief Polymorphic Engine Entry Point and ChaCha20 CSPRNG implementation.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <camkes.h>

/* Shared memory regions defined in CAmkES */
extern void *guest_ram; 
extern void *gen_buf;   

/* --- ChaCha20 CSPRNG Implementation --- */

#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) \
    a += b; d ^= a; d = ROTL(d, 16); \
    c += d; b ^= c; b = ROTL(b, 12); \
    a += b; d ^= a; d = ROTL(b, 8);  \
    c += d; b ^= c; b = ROTL(b, 7);

static uint32_t chacha_state[16];
static uint32_t chacha_stream[16];
static int chacha_index = 16;
static int is_csprng_ready = 0;

static void chacha_permute_block(uint32_t out[16], uint32_t const in[16]) {
    int i;
    for (i = 0; i < 16; ++i) out[i] = in[i];
    for (i = 0; i < 10; ++i) { 
        QR(out[0], out[4], out[ 8], out[12]);
        QR(out[1], out[5], out[ 9], out[13]);
        QR(out[2], out[6], out[10], out[14]);
        QR(out[3], out[7], out[11], out[15]);
        QR(out[0], out[5], out[10], out[15]);
        QR(out[1], out[6], out[11], out[12]);
        QR(out[2], out[7], out[ 8], out[13]);
        QR(out[3], out[4], out[ 9], out[14]);
    }
    for (i = 0; i < 16; ++i) out[i] += in[i];
}

static void initialize_hardware_entropy(void) {
    /* Standard ChaCha20 Constants */
    chacha_state[0] = 0x61707865; chacha_state[1] = 0x3320646e;
    chacha_state[2] = 0x79622d32; chacha_state[3] = 0x6b206574;
    
    for(int i = 4; i < 16; i++) {
        uint32_t lo, hi;
        /* Gather entropy from CPU Timestamp Counter */
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
        chacha_state[i] = lo ^ hi ^ (i * 0x9E3779B9);
    }
    is_csprng_ready = 1;
    printf("[GEN] INFO: Internal ChaCha20 CSPRNG initialized with hardware entropy.\n");
}

static uint32_t request_secure_random(void) {
    if (!is_csprng_ready) initialize_hardware_entropy();
    if (chacha_index == 16) {
        chacha_permute_block(chacha_stream, chacha_state);
        chacha_state[12]++; 
        chacha_index = 0;
    }
    return chacha_stream[chacha_index++];
}

static void stream_emit_32(uint8_t **stream, uint32_t val) {
    memcpy(*stream, &val, 4);
    *stream += 4;
}

/* --- RPC Interface Implementation --- */

/**
 * @brief Synthesizes a new polymorphic x86_64 variant.
 */
void gen_api_request_generation(uint64_t base_addr, int variant_id, int complexity) {
    printf("[GEN] ACTION: Synthesizing variant %d (Complexity: %d)...\n", variant_id, complexity);
    
    uint8_t *ram_ptr = (uint8_t *)guest_ram;
    uint8_t *cursor = ram_ptr;
    uint32_t logic_payload = 0xDEADBEEF;
    
    /* 1. Junk code insertion (MTD strategy) */
    int junk_len = 5 + (request_secure_random() % 25);
    for (int i = 0; i < junk_len; i++) {
        uint32_t r = request_secure_random() % 4;
        if (r == 0)      *cursor++ = 0x90; // NOP
        else if (r == 1) *cursor++ = 0xFC; // CLD
        else if (r == 2) *cursor++ = 0xF8; // CLC
        else { *cursor++ = 0x87; *cursor++ = 0xC9; } // XCHG ECX, ECX
    }

    /* 2. Logic Masking */
    uint32_t mask = request_secure_random();
    uint32_t obfuscated_val = logic_payload ^ mask;
    
    /* x86_64 Opcode construction */
    *cursor++ = 0x31; *cursor++ = 0xC0;               // XOR EAX, EAX
    *cursor++ = 0x05; stream_emit_32(&cursor, mask);   // ADD EAX, mask
    *cursor++ = 0x35; stream_emit_32(&cursor, obfuscated_val); // XOR EAX, obfuscated_val
    *cursor++ = 0xC3;                                  // RET

    int total_size = cursor - ram_ptr;
    memcpy(gen_buf, guest_ram, total_size);
    
    printf("[GEN] SUCCESS: Binary variant generated (%d bytes). Notifying Repository.\n", total_size);
    
    /* Signal Repository to register the new variant */
    repo_write_api_add_variant(variant_id, total_size);
}

void hw_tick_handle(void) { }
int run(void) { return 0; }
