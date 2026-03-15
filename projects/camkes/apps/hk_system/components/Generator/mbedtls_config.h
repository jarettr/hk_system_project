#ifndef MBEDTLS_CONFIG_BAREMETAL_H
#define MBEDTLS_CONFIG_BAREMETAL_H

/* Базовые настройки для микроядра (нет libc, нет ОС) */
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

/* ЖЕСТКИЙ ПРИКАЗ: Использовать только нашу аппаратную энтропию (RDTSC) */
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

/* Включаем модули AES, SHA-256 и сам CSPRNG */
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_AES_C
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C

#include "mbedtls/check_config.h"
#endif
