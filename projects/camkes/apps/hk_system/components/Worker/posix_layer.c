/**
 * @file posix_layer.c
 * @brief Custom POSIX Abstraction Layer for the Sandbox.
 * Prevents naming collisions with standard library implementations (e.g., libsel4muslcsys)
 * while routing IO operations through secure CAmkES RPC channels.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <camkes.h>

/**
 * @brief Proxies write operations to the VFS server.
 */
long hk_posix_write(int fd, const void *buf, size_t count) {
    /* The actual payload is transferred via the fs_buffer Shared Data region.
     * The RPC call only notifies the VFS server of the transaction size.
     */
    return (long)fs_api_write(fd, (int)count);
}

int open_file(const char *path) {
    return fs_api_open(path, 0);
}

int read_file(int fd, char *buffer, int max_len) {
    return fs_api_read(fd, max_len);
}

/**
 * @brief Secure formatted print router.
 */
void print_posix(const char *format, ...) {
    char buffer[256];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    hk_posix_write(1, buffer, strlen(buffer));
}
