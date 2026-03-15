#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <camkes.h>

/*
 * ПЕРЕХВАТ СИСТЕМНЫХ ВЫЗОВОВ (Слой адаптации)
 * Мы переименовали функцию, чтобы избежать конфликта с libsel4muslcsys.
 */

long hk_posix_write(int fd, const void *buf, size_t count) {
    /* * В CAmkES интерфейс PosixAPI.write принимает (int fd, int count).
     * Сами данные уже должны лежать в Shared Memory (fs_buffer).
     */
    return (long)fs_write(fd, (int)count);
}

int open_file(const char *path) {
    return fs_open(path, 0);
}

int read_file(int fd, char *buffer, int max_len) {
    return fs_read(fd, max_len);
}

void print_posix(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Используем наш переименованный вызов
    hk_posix_write(1, buffer, strlen(buffer));
}
