#ifndef _SSL_FN_H
#define _SSL_FN_H

#include <openssl/evp.h>

/**
 * @brief Create checksum of the file pointed to by src
 *
 * @return NULL - failure
 */
uint8_t *check_sha3_512_from_file(const char *src,
								  uint32_t *retLen);

/**
 * @brief Begin or update checksum procedure
 *
 * The EVP_MD_CTX will not be allocated if data or size is 0
 *
 * @return NULL - failure
 */
EVP_MD_CTX *update_sha3_512(const void *data,
							uint32_t size,
							EVP_MD_CTX *md_ctx = NULL);

/**
 * @brief Finalize checksum procedure
 *
 * The EVP_MD_CTX will always be free'd
 *
 * @return NULL - failure
 */
uint8_t *finish_sha3_512(uint32_t *retLen,
						 EVP_MD_CTX *md_ctx);

/**
 * @brief Finalize checksum procedure
 *
 * The EVP_MD_CTX will always be free'd
 *
 * @return NULL - failure
 */
uint8_t *finish_sha3_512(const void *data,
						 uint32_t size,
						 uint32_t *retLen,
						 EVP_MD_CTX *md_ctx = NULL);

bool sha512_cmp      (uint8_t *left, uint8_t *right);
bool sha512_cmp_free (uint8_t *left, uint8_t *target);
bool sha512_copy     (uint8_t *dest, uint8_t *src,    uint32_t shaLen);
bool sha512_copy_free(uint8_t *dest, uint8_t *target, uint32_t shaLen);

#endif //_SSL_FN_H
