#include "alibio.h"
#include "ssl_fn.h"

#include <openssl/evp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF4K 0x1000
//#define BUF1M 0x100000

uint8_t *check_sha3_512_from_file(const char *src)
{
	EVP_MD_CTX *md_ctx = NULL;
	FILE *fp = fopen(src, "rb");
	uint32_t data_len, file_len = getFilesize(fp);
	uint32_t md_len;
	uint8_t *md_val = NULL;
	uint8_t data[BUF4K];

	if (!fp || !(md_ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!(md_val = (uint8_t *) malloc(EVP_MAX_MD_SIZE))) goto cleanup;
	memset(md_val, 0, EVP_MAX_MD_SIZE);
	EVP_DigestInit_ex(md_ctx, EVP_sha3_512(), NULL);

	while (file_len > 0)
	{
		data_len = fread(data, 1, BUF4K, fp);
		file_len -= data_len;
		EVP_DigestUpdate(md_ctx, data, data_len);
	}
	EVP_DigestFinal_ex(md_ctx, md_val, &md_len);

cleanup:
	if (fp)     fclose(fp);
	if (md_ctx) EVP_MD_CTX_free(md_ctx);
	return md_val;
}

uint8_t *check_sha3_512(const uint8_t *data, uint32_t size)
{
	EVP_MD_CTX *md_ctx = NULL;
	uint32_t md_len;
	uint8_t *md_val = NULL;

	if (!(md_ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!(md_val = (uint8_t *) malloc(EVP_MAX_MD_SIZE))) goto cleanup;
	memset(md_val, 0, EVP_MAX_MD_SIZE);
	EVP_DigestInit_ex(md_ctx, EVP_sha3_512(), NULL);
	EVP_DigestUpdate(md_ctx, data, size);
	EVP_DigestFinal_ex(md_ctx, md_val, &md_len);

cleanup:
	if (md_ctx) EVP_MD_CTX_free(md_ctx);
	return md_val;
}
