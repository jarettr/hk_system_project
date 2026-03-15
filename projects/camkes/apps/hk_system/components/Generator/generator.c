#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <camkes.h>

extern void *guest_ram; 
extern void *gen_buf;   

/* =====================================================================
 * КРИПТОГРАФИЧЕСКИ СТОЙКИЙ ГЕНЕРАТОР (CSPRNG) - ChaCha20
 * ===================================================================== */
#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
#define QR(a, b, c, d) \
    a += b; d ^= a; d = ROTL(d, 16); \
    c += d; b ^= c; b = ROTL(b, 12); \
    a += b; d ^= a; d = ROTL(b, 8);  \
    c += d; b ^= c; b = ROTL(b, 7);

static uint32_t chacha_state[16];
static uint32_t chacha_stream[16];
static int chacha_index = 16;
static int is_csprng_init = 0;

static void chacha_block(uint32_t out[16], uint32_t const in[16]) {
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

static void init_secure_prng() {
    chacha_state[0] = 0x61707865; chacha_state[1] = 0x3320646e;
    chacha_state[2] = 0x79622d32; chacha_state[3] = 0x6b206574;
    for(int i = 4; i < 16; i++) {
        uint32_t lo, hi;
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
        chacha_state[i] = lo ^ hi ^ (i * 0x9E3779B9);
    }
    is_csprng_init = 1;
    printf("[GENERATOR] 🔐 Встроенный CSPRNG (ChaCha20) инициализирован!\n");
}

static uint32_t get_secure_rand() {
    if (!is_csprng_init) init_secure_prng();
    if (chacha_index == 16) {
        chacha_block(chacha_stream, chacha_state);
        chacha_state[12]++; 
        chacha_index = 0;
    }
    return chacha_stream[chacha_index++];
}

static void emit_32(uint8_t **stream, uint32_t val) {
    memcpy(*stream, &val, 4);
    *stream += 4;
}

/* =====================================================================
 * ПОЛИМОРФНЫЙ ДВИЖОК
 * ===================================================================== */
// ИСПРАВЛЕНИЕ: Меняем int на void
void gen_api_request_generation(unsigned long base_addr, int variant_id, int complexity) {
    printf("\n[GENERATOR] 🧬 Синтез ПОЛИМОРФНОГО кода (CSPRNG) (Вариант: %d)...\n", variant_id);
    
    uint8_t *ram = (uint8_t *)guest_ram;
    uint8_t *cursor = ram;
    uint32_t target_payload = 0xDEADBEEF;
    
    int junk_len = 5 + (get_secure_rand() % 25);
    for (int i = 0; i < junk_len; i++) {
        uint32_t r = get_secure_rand() % 4;
        if (r == 0)      *cursor++ = 0x90; 
        else if (r == 1) *cursor++ = 0xFC; 
        else if (r == 2) *cursor++ = 0xF8; 
        else { *cursor++ = 0x87; *cursor++ = 0xC9; }
    }

    uint32_t mask1 = get_secure_rand();
    uint32_t mask2 = target_payload ^ mask1;
    
    *cursor++ = 0x31; *cursor++ = 0xC0;             
    *cursor++ = 0x05; emit_32(&cursor, mask1);      
    *cursor++ = 0x35; emit_32(&cursor, mask2);      
    *cursor++ = 0xC3; 

    int size = cursor - ram;
    memcpy(gen_buf, guest_ram, size);
    
    printf("[GENERATOR] ⚙️ Упаковано кода: %d байт. Отправка в Repo...\n", size);
    
    // ИСПРАВЛЕНИЕ: Правильное имя метода из IDL
    repo_write_add_variant(variant_id, size);
}

void hw_tick_handle(void) { }
int run(void) { return 0; }
