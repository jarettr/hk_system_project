#ifndef MBEDTLS_CONFIG_BAREMETAL_H
#define MBEDTLS_CONFIG_BAREMETAL_H

/* Системная конфигурация */
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS

/* Криптография (только то, что нам нужно для подписи) */
#define MBEDTLS_SHA256_C
#define MBEDTLS_MD_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_RSA_C
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_BIGNUM_C

#include "mbedtls/check_config.h"
#endif
