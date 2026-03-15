#include <stdio.h>
#include <string.h>
#include <camkes.h>

int run(void) {
    // Ждем, пока логи загрузки улягутся
    for(volatile int i=0; i < 60000000; i++);

    printf("\n\n============================================\n");
    printf("🛡️  HEURISTIC OS - КОНСОЛЬ УПРАВЛЕНИЯ ЗАПУЩЕНА\n");
    printf("============================================\n");

    while(1) {
        printf("\n[ADMIN] Отправка команды FORCE_MUTATION к Диспетчеру...\n");
        
        // Вызываем ТОЛЬКО Диспетчера. Он сам решит, как дергать JIT и MMU.
        admin_api_force_mutation();
        
        // Пауза 10 секунд для чистоты логов
        for(volatile int i=0; i < 500000000; i++);
    }
    return 0;
}
