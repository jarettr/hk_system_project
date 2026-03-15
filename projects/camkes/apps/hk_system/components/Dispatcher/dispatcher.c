#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <camkes.h>

static int total_mutations = 0;
static int active_variant_id = 0;

extern void *guest_ram;    
extern void *worker_arena; 
extern void *repo_storage;

void execute_mmu_hot_swap(int variant_id) {
    printf("[DISPATCHER] 🔄 Hot-Swap: Валидация JIT-кода...\n");
    
    // 1. Читаем метаданные из Репозитория
    uint32_t *meta = (uint32_t *)repo_storage;
    int rep_vid = meta[0];
    int size = meta[1];
    uint32_t expected_sig = meta[2];
    
    if (rep_vid != variant_id) {
        printf("[DISPATCHER] ❌ Ошибка: В репозитории нет нужного варианта!\n");
        return;
    }

    // 2. Считаем хэш того кода, который лежит в JIT-буфере
    uint8_t *code = (uint8_t *)guest_ram;
    uint32_t hash = 0;
    for(int i = 0; i < size; i++) {
        hash += code[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    printf("[DISPATCHER] 🔎 Код в JIT-буфере хеширован: 0x%X\n", hash);
    
    // 3. Отправляем в TPM на верификацию! (Используем наш новый RPC-канал)
    int is_valid = tpm_verify_verify_signature(hash, expected_sig);
    
    if (is_valid) {
        // 4. Если TPM сказал ОК - делаем Hot-Swap
        memcpy(worker_arena, guest_ram, 4096);
        active_variant_id = variant_id;
        total_mutations++;
        printf("[DISPATCHER] ✅ Код успешно верифицирован TPM и заменен в памяти Воркера!\n");
    } else {
        printf("[DISPATCHER] 🚨 КРИТИЧЕСКАЯ УГРОЗА: TPM отверг подпись. Hot-Swap заблокирован!\n");
    }
}

void admin_api_force_mutation(void) {
    int next_id = total_mutations + 1;
    printf("\n======================================================\n");
    printf("[ADMIN] ⚡ Инициирована защищенная JIT-транзакция (ID: %d)\n", next_id);
    
    // 1. Генерируем, хешируем, подписываем
    gen_request_generation((unsigned long)0x1000, next_id, 5);
    
    // 2. Верифицируем через TPM и вливаем в Воркера
    execute_mmu_hot_swap(next_id);
    
    // 3. ЗАПУСК ЗАДАЧИ НА ВОРКЕРЕ
    printf("[DISPATCHER] 📡 Отправка команды Воркеру на исполнение кода...\n");
    int res = worker_ctrl_cmd(1, 0); // Вызов через RPC
    printf("[DISPATCHER] 🏆 Итоговый ответ от Воркера получен ядром: 0x%X\n", res);
    printf("======================================================\n\n");
}

void admin_api_print_status(void) {
    printf("\n--- [SECURITY TELEMETRY] ---\n");
    printf("  Active Variant: %d\n", active_variant_id);
    printf("  Total Swaps:    %d\n", total_mutations);
    printf("----------------------------\n");
}

void hw_tick_handle(void) { }
int run(void) { return 0; }
