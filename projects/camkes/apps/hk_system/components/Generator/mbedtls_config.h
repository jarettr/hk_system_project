#ifndef MBEDTLS_CONFIG_BAREMETAL_H
#define MBEDTLS_CONFIG_BAREMETAL_H

/**
 * @brief MbedTLS configuration for seL4 Trusted Execution Environment.
 * Optimized for bare-metal execution without standard libc.
 */

#define MBEDTLS_HAVE_ASM
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

/* Hardware-root-of-trust: Use internal entropy only */
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

/* Core Security Modules */
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_AES_C
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C

#include "mbedtls/check_config.h"

#endif
