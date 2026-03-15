#ifndef HK_SDK_H
#define HK_SDK_H

/* * SDK Операционной Системы "Живое Ядро" (Heuristic Kernel)
 * Набор макросов для управления динамической мутацией кода.
 */

// 1. HK_MUTATE: Разрешить максимальную мутацию функции (вставка мусора, перестановка блоков)
#define HK_MUTATE __attribute__((annotate("hk_mutate")))

// 2. HK_NO_MUTATE: Запретить мутацию. Использовать для критических секций реального времени.
#define HK_NO_MUTATE __attribute__((annotate("hk_no_mutate")))

// 3. HK_MUTATE_RO: Мутировать только данные/переменные, но не менять граф выполнения.
#define HK_MUTATE_RO __attribute__((annotate("hk_mutate_ro")))

/*
 * API для регистрации колбэков (ТЗ 8.2)
 * Позволяет модулю сохранить свое состояние ПЕРЕД горячей заменой 
 * и восстановить ПОСЛЕ загрузки нового варианта.
 */
typedef void (*hk_callback_t)(void);

// Заглушки функций, которые будут реализованы в системной библиотеке
void hk_register_pre_mutation_callback(hk_callback_t callback);
void hk_register_post_mutation_callback(hk_callback_t callback);

#endif // HK_SDK_H