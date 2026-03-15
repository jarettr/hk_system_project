#include <stdio.h>
#include <camkes.h>

// Базовый квант из ТЗ (500 мс)
static int current_quantum_ms = 500; 

// Функция, вызываемая Диспетчером через IPC
void ctrl_set_quantum(int milliseconds) {
    printf("    [SCHEDULER] ⚠️ Команда от Монитора: Изменение кванта времени с %d мс на %d мс!\n", 
           current_quantum_ms, milliseconds);
    current_quantum_ms = milliseconds;
}

int run(void) {
    printf("[SCHEDULER] Аппаратный таймер запущен. Базовый квант = %d мс.\n", current_quantum_ms);
    
    while (1) {
        // Простая эмуляция задержки, зависящая от current_quantum_ms
        // (В реальности здесь программируется PIT/HPET контроллер)
        volatile int delay = current_quantum_ms * 100000; 
        for(volatile int i = 0; i < delay; i++); 
        
        hw_tick_emit(); 
    }
    
    return 0;
}