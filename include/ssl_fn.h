#ifndef _SSL_FN_H
#define _SSL_FN_H

/**
 * @brief Create checksum of the file pointed to by src
 *
 * @return NULL - failure
 */
uint8_t *check_sha3_512_from_file(const char *src, uint32_t *retLen);

/**
 * @brief Create checksum of the array
 *
 * @return NULL - failure
 */
uint8_t *check_sha3_512(const void *data, uint32_t size, uint32_t *retLen);

#endif //_SSL_FN_H
