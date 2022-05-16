#include "alib_io.h"
#include "ssl_fn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: remove this check once github CI is at OpenSSL 3.0
#if OPENSSL_VERSION_MAJOR < 3
void *EVP_PKEY_CTX_new_from_name(void *a, void *b, void *c){return NULL;}
void *EVP_PKEY_Q_keygen(void *a, void *b, void *c){return NULL;}
#endif

uint8_t *check_sha3_512_from_file(const char *src, uint32_t *retLen)
{
	if (!src || !retLen) return NULL;
	EVP_MD_CTX *md_ctx = NULL;
	FILE *fp = fopen(src, "rb");
	uint32_t data_len, file_len = getFilesize(fp);
	uint8_t *data = (uint8_t *) malloc(BUF4K);
	uint8_t *ret = NULL;

	if (!data || !fp) goto cleanup;
	while (file_len > 0)
	{
		memset(data, 0, BUF4K);
		data_len = fread(data, 1, BUF4K, fp);
		file_len -= data_len;
		md_ctx = update_sha3_512(data, data_len, md_ctx);
	}
	ret = finish_sha3_512(retLen, md_ctx);

cleanup:
	if (data)   free(data);
	if (fp)     fclose(fp);
	return ret;
}

EVP_MD_CTX *update_sha3_512(const void *data, uint32_t size, EVP_MD_CTX *md_ctx)
{
	EVP_MD_CTX *local_ctx = md_ctx;

	if (!data || !size) goto cleanup;
	if (!local_ctx && !(local_ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!md_ctx) EVP_DigestInit_ex(local_ctx, EVP_sha3_512(), NULL);
	EVP_DigestUpdate(local_ctx, data, size);

cleanup:
	return local_ctx;
}

uint8_t *finish_sha3_512(uint32_t *retLen, EVP_MD_CTX *md_ctx)
{
	uint8_t *md_val = NULL;

	if (!retLen || !md_ctx) goto cleanup;
	if (!(md_val = (uint8_t *) calloc(EVP_MAX_MD_SIZE, 1))) goto cleanup;
	EVP_DigestFinal_ex(md_ctx, md_val, retLen);

cleanup:
	if (md_ctx) EVP_MD_CTX_free(md_ctx);
	return md_val;
}

uint8_t *finish_sha3_512(const void *data, uint32_t size, uint32_t *retLen, EVP_MD_CTX *md_ctx)
{
	return finish_sha3_512(retLen, update_sha3_512(data, size, md_ctx));
}

bool sha512_cmp(uint8_t *left, uint8_t *right)
{
	if (!left || !right) return false;
	return 0 == memcmp(left, right, SHA512_LEN);
}

bool sha512_cmp_free(uint8_t *left, uint8_t *target)
{
	bool ret = sha512_cmp(left, target);
	if (target) free(target);
	return ret;
}

bool sha512_copy(uint8_t *dest, uint8_t *src, uint32_t shaLen)
{
	if (!src || shaLen != SHA512_LEN)
	{
		printf("SHA3-512 for crc failed. Bytes expected=%d, actual=%u\n", SHA512_LEN, shaLen);
		return false;
	}
	memcpy(dest, src, SHA512_LEN);
	return true;
}

bool sha512_copy_free(uint8_t *dest, uint8_t *target, uint32_t shaLen)
{
	bool ret = sha512_copy(dest, target, shaLen);
	if (target) free(target);
	return ret;
}
