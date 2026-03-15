#include <stdio.h>
#include <string.h>
#include <camkes.h>

// Эмуляция сетевого стека (заглушки для линковки)
int lwip_socket(int domain, int type, int protocol) { return 101; }
int lwip_send(int sockfd, const char *buf, int len, int flags) { return len; }

static int disk_offset = 0; 
static const int DISK_SIZE = 1048576;

#define LOG_FD 5
#define SOCKET_FD_BASE 100 

// -------------------------------------------------------------------
// POSIX: ФАЙЛОВАЯ СИСТЕМА (Адаптировано под CAmkES IPC)
// -------------------------------------------------------------------

int posix_api_open(const char *path, int flags) {
    if (strcmp(path, "/var/log/worker.log") == 0) {
        printf("    [VFS-SERVER] Открыт файл на виртуальном диске: %s (FD=%d)\n", path, LOG_FD);
        return LOG_FD; 
    }
    return -1;
}

// ИСПРАВЛЕНО: Аргументы приведены к int, данные читаются из virtual_disk_arena
int posix_api_read(int fd, int count) {
    if (fd == LOG_FD && disk_offset > 0) {
        int read_size = (count < disk_offset) ? count : disk_offset;
        // В реальной системе данные уже лежат в virtual_disk_arena
        printf("    [VFS-SERVER] Чтение %d байт из лога\n", read_size);
        return read_size;
    }
    return 0;
}

// ИСПРАВЛЕНО: Аргументы приведены к int, данные берутся из virtual_disk_arena
int posix_api_write(int fd, int count) {
    // 1. Запись на виртуальный диск
    if (fd == LOG_FD) {
        if (disk_offset + count >= DISK_SIZE) {
            printf("    [VFS-SERVER] ❌ ОШИБКА: Виртуальный диск переполнен!\n");
            return -1;
        }
        // Данные уже находятся в virtual_disk_arena, нам нужно только сместить указатель
        disk_offset += count; 
        printf("    [VFS-SERVER] Записано в лог: %d байт (Всего: %d)\n", count, disk_offset);
        return count;
    }
    
    // 2. Стандартный вывод
    if (fd == 1 || fd == 2) { 
        // Мы не можем напечатать buf, так как получили только размер.
        // Но мы знаем, что Воркер положил данные в Shared Memory.
        printf("    [APP-STDOUT]: (Shared Memory Write, size: %d)\n", count);
        return count;
    }

    // 3. Отправка в сеть
    if (fd >= SOCKET_FD_BASE) {
        printf("    [VFS-SERVER] Маршрутизация %d байт в LwIP стек...\n", count);
        return count;
    }

    return -1;
}

int posix_api_close(int fd) {
    if (fd == LOG_FD) {
        printf("    [VFS-SERVER] Файл логов закрыт. Занято: %d байт.\n", disk_offset);
    }
    return 0;
}

// Точка входа
int run(void) {
    printf("[VFS-SERVER] Инициализация подсистем ввода-вывода...\n");
    
    // Очищаем арену диска
    if (virtual_disk_arena != NULL) {
        memset((void*)virtual_disk_arena, 0, DISK_SIZE);
        printf("[VFS-SERVER] RAM-диск (1MB) смонтирован.\n");
    }
    
    printf("[VFS-SERVER] Сетевой стек LwIP готов к работе.\n");

    return 0;
}
