/**
 * @file vfs_server.c
 * @brief Virtual File System and Network Proxy Server.
 * Emulates POSIX-compliant IO operations and routes them to isolated 
 * subsystems (e.g., LwIP network stack, RAM disk) via CAmkES IPC.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <camkes.h>

/* Extern declaration for the CAmkES dataport */
extern void *virtual_disk_arena;

/* --- Network Stack Stubs (LwIP Proxy) --- */
int lwip_socket(int domain, int type, int protocol) { return 101; }
int lwip_send(int sockfd, const char *buf, int len, int flags) { return len; }

/* --- VFS Configuration --- */
static uint32_t disk_offset = 0; 
static const uint32_t DISK_SIZE = 1048576; // 1 MB

#define LOG_FD 5
#define SOCKET_FD_BASE 100 

/* =====================================================================
 * POSIX API IMPLEMENTATION (Proxied via RPC)
 * ===================================================================== */

int posix_api_open(const char *path, int flags) {
    if (strcmp(path, "/var/log/worker.log") == 0) {
        printf("[VFS] INFO: Virtual disk file opened: %s (FD=%d)\n", path, LOG_FD);
        return LOG_FD; 
    }
    return -1;
}

int posix_api_read(int fd, int count) {
    if (fd == LOG_FD && disk_offset > 0) {
        int read_size = ((uint32_t)count < disk_offset) ? count : (int)disk_offset;
        /* In a real implementation, data is fetched from virtual_disk_arena */
        printf("[VFS] IO: Reading %d bytes from secure log.\n", read_size);
        return read_size;
    }
    return 0;
}

int posix_api_write(int fd, int count) {
    /* 1. Write to Virtual RAM Disk */
    if (fd == LOG_FD) {
        if (disk_offset + (uint32_t)count >= DISK_SIZE) {
            printf("[VFS] ERROR: Virtual disk quota exceeded (OOM)!\n");
            return -1;
        }
        /* Payload is already in virtual_disk_arena; advance offset */
        disk_offset += (uint32_t)count; 
        printf("[VFS] IO: Wrote %d bytes to log (Total: %u bytes).\n", count, disk_offset);
        return count;
    }
    
    /* 2. Standard Output / Error Routing */
    if (fd == 1 || fd == 2) { 
        /* Payload resides in Shared Memory, printing metadata only */
        printf("[VFS] STDOUT_PROXY: (Shared Memory Write, size: %d bytes)\n", count);
        return count;
    }

    /* 3. Network Routing (LwIP) */
    if (fd >= SOCKET_FD_BASE) {
        printf("[VFS] NET: Routing %d bytes to LwIP stack (FD=%d)...\n", count, fd);
        return count;
    }

    return -1;
}

int posix_api_close(int fd) {
    if (fd == LOG_FD) {
        printf("[VFS] INFO: Log file closed. Storage used: %u bytes.\n", disk_offset);
    }
    return 0;
}

/* --- Component Entry Point --- */
int run(void) {
    printf("[VFS] INFO: Initializing isolated I/O subsystems...\n");
    
    /* Validate dataport mapping before zeroing memory */
    if (virtual_disk_arena != NULL) {
        memset((void*)virtual_disk_arena, 0, DISK_SIZE);
        printf("[VFS] SUCCESS: Volatile RAM disk (1MB) mounted and zeroed.\n");
    } else {
        printf("[VFS] ERROR: Failed to map virtual_disk_arena dataport!\n");
    }
    
    printf("[VFS] SUCCESS: LwIP Network proxy ready.\n");

    return 0;
}
