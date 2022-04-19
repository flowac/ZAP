#include "alibio.h"
#include "ssl_fn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *check_sha3_512_from_file(const char *src, uint32_t *retLen)
{
	if (!src || !retLen) return NULL;
	EVP_MD_CTX *md_ctx = NULL;
	FILE *fp = fopen(src, "rb");
	uint32_t data_len, file_len = getFilesize(fp);
	uint8_t *md_val = NULL;
	uint8_t *data = (uint8_t *) malloc(BUF4K);

	if (!data || !fp || !(md_ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!(md_val = (uint8_t *) calloc(EVP_MAX_MD_SIZE, 1))) goto cleanup;
	EVP_DigestInit_ex(md_ctx, EVP_sha3_512(), NULL);

	while (file_len > 0)
	{
		memset(data, 0, BUF4K);
		data_len = fread(data, 1, BUF4K, fp);
		file_len -= data_len;
		EVP_DigestUpdate(md_ctx, data, data_len);
	}
	EVP_DigestFinal_ex(md_ctx, md_val, retLen);

cleanup:
	if (data)   free(data);
	if (fp)     fclose(fp);
	if (md_ctx) EVP_MD_CTX_free(md_ctx);
	return md_val;
}

EVP_MD_CTX *update_sha3_512(const void *data, uint32_t size, EVP_MD_CTX *md_ctx)
{
	EVP_MD_CTX *local_ctx = md_ctx;

	if (!data) goto cleanup;
	if (!local_ctx && !(local_ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!md_ctx) EVP_DigestInit_ex(local_ctx, EVP_sha3_512(), NULL);
	EVP_DigestUpdate(local_ctx, data, size);

	return local_ctx;
cleanup:
	if (local_ctx) EVP_MD_CTX_free(local_ctx);
	return NULL;
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
