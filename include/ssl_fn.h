#ifndef _SSL_FN_H
#define _SSL_FN_H

#include <openssl/evp.h>

// TODO: remove this check once github CI is at OpenSSL 3.0
#include <openssl/opensslv.h>
#if OPENSSL_VERSION_MAJOR < 3
void *EVP_PKEY_CTX_new_from_name(void *a, void *b, void *c);
void *EVP_PKEY_Q_keygen(void *a, void *b, void *c);
#endif

/**
 * @brief Create checksum of the file pointed to by src
 *
 * @return NULL - failure
 */
uint8_t *check_sha3_from_file(const char *src, uint32_t *retLen);

/**
 * @brief Begin or update checksum procedure
 *
 * The EVP_MD_CTX will not be allocated if data or size is 0
 *
 * @return NULL - failure
 */
EVP_MD_CTX *update_sha3(const void *data, uint32_t size, EVP_MD_CTX *md_ctx = NULL);
EVP_MD_CTX *update_shake(const void *data, uint32_t size, EVP_MD_CTX *md_ctx = NULL);

/**
 * @brief Finalize checksum procedure
 *
 * The EVP_MD_CTX will always be free'd and set to NULL
 *
 * @return NULL - failure
 */
uint8_t *finish_sha3(uint32_t *retLen, EVP_MD_CTX **md_ctx);
uint8_t *finish_shake(uint32_t *retLen, EVP_MD_CTX **md_ctx);

/**
 * @brief Finalize checksum procedure
 *
 * The EVP_MD_CTX will always be free'd
 *
 * @return NULL - failure
 */
uint8_t *finish_sha3(const void *data, uint32_t size, uint32_t *retLen, EVP_MD_CTX *md_ctx = NULL);
uint8_t *finish_shake(const void *data, uint32_t size, uint32_t *retLen, EVP_MD_CTX *md_ctx = NULL);

bool sha3_cmp       (uint8_t *left, uint8_t *right);
bool sha3_cmp_free  (uint8_t *left, uint8_t *target);
bool sha3_copy      (uint8_t *dest, uint8_t *src,    uint32_t shaLen);
bool sha3_copy_free (uint8_t *dest, uint8_t *target, uint32_t shaLen);
bool shake_cmp      (uint8_t *left, uint8_t *right);
bool shake_cmp_free (uint8_t *left, uint8_t *target);
bool shake_copy     (uint8_t *dest, uint8_t *src,    uint32_t shaLen);
bool shake_copy_free(uint8_t *dest, uint8_t *target, uint32_t shaLen);

#endif //_SSL_FN_H
