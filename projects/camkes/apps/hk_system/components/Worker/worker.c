#include <stdio.h>
#include <stdint.h>
#include <camkes.h>

extern void *executable_arena;

/* =====================================================================
 * ИНТЕРФЕЙС ДЛЯ ВЕРИФИКАТОРА (То, что потерял линковщик)
 * ===================================================================== */
uint32_t verifier_target_get_hash(void) {
    uint8_t *ram = (uint8_t *)executable_arena;
    uint32_t hash = 0;
    
    // Считаем хэш текущей памяти, чтобы Верификатор не ругался
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
 * ИСПОЛНЕНИЕ ПОЛИМОРФНОГО КОДА (JIT)
 * ===================================================================== */
int ctrl_cmd(int cmd, int arg) {
    if (cmd == 1) {
        printf("    [WORKER] 🚀 Получен приказ на исполнение JIT-арены...\n");
        
        // Магия C: превращаем адрес памяти (Dataport) в вызываемую функцию
        uint32_t (*jit_task)(void) = (uint32_t (*)(void))executable_arena;
        
        // Вызываем сгенерированный полиморфный ассемблерный код!
        uint32_t result = jit_task();
        
        printf("    [WORKER] 🎯 Задача выполнена аппаратно. Результат: 0x%X\n", result);
        return result;
    }
    return 0;
}

void hw_tick_handle(void) { }
int run(void) { 
    // Воркер сидит тихо и ждет команд от Диспетчера
    return 0; 
}
