#ifndef _SSL_FN_H
#define _SSL_FN_H

/**
 * @brief Create checksum of the file pointed to by src
 *
 * @return NULL - failure
 */
uint8_t *check_sha3_512_from_file(const char *src);

/**
 * @brief Create checksum of the array
 *
 * @return NULL - failure
 */
uint8_t *check_sha3_512(const uint8_t *data, uint32_t size);

#endif //_SSL_FN_H
