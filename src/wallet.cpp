#include <stdlib.h>
#include <string.h>

#include "main_lib.h"
#include "file_io.h"
#include "wallet.h"
#include "ssl_fn.h"
#include "time_fn.h"

#include <openssl/obj_mac.h>
#include <openssl/ec.h>

// TODO: make the seed work
bool newWallet(uint8_t pub[ED448_LEN], uint8_t priv[ED448_LEN], const char *seed)
{
	bool ret = false;
	size_t pubLen = ED448_LEN, privLen = ED448_LEN;
	EVP_PKEY *key = NULL;
	EVP_PKEY_CTX *ctx = NULL;

	if (!(ctx = EVP_PKEY_CTX_new_from_name(NULL, "ED448", NULL))) goto cleanup;
	if (!(key = EVP_PKEY_Q_keygen(NULL, NULL, "ED448"))) goto cleanup;

	if (EVP_PKEY_get_raw_public_key(key, pub, &pubLen) < 1) goto cleanup;
	if (EVP_PKEY_get_raw_private_key(key, priv, &privLen) < 1) goto cleanup;
//	printf("pub[%lu] priv[%lu]\n", pubLen, privLen);
//	printBytes(stdout, pub, pubLen, "\n");
//	printBytes(stdout, priv, privLen, "\n");
	ret = true;

cleanup:
	if (ctx) EVP_PKEY_CTX_free(ctx);
	if (key) EVP_PKEY_free(key);
	return ret;
}

// TODO: refactor and move generic signing into ssl_fn.c
bool sendToAddress(uint8_t dest[ED448_LEN], uint8_t src[ED448_LEN], uint8_t sig[ED448_SIG_LEN], uint64_t deci, uint16_t frac)
{
	bool ret = false;
	size_t sigLen = ED448_SIG_LEN, msgLen = 10;
	uint8_t msg[10];
	EVP_MD_CTX *ctx = NULL;
	EVP_PKEY *key = NULL;

	if (!(ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!(key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED448, NULL, src, ED448_LEN))) goto cleanup;

	u64Packer(msg, deci);
	msg[8] = frac | MAX_U8;
	msg[9] = frac >> 8;

	if (EVP_DigestSignInit(ctx, NULL, NULL, NULL, key) < 1) goto cleanup;
	if (EVP_DigestSign(ctx, sig, &sigLen, msg, msgLen) < 1) goto cleanup;

//	printf("sig[%lu]\n", sigLen);
//	printBytes(stdout, sig, sigLen, "\n");
	ret = true;

cleanup:
	if (ctx) EVP_MD_CTX_free(ctx);
	if (key) EVP_PKEY_free(key);
	return ret;
}

bool verifyMessage(uint8_t pub[ED448_LEN], uint8_t sig[ED448_SIG_LEN], uint8_t *msg, uint64_t msgLen)
{
	bool ret = false;
	EVP_MD_CTX *ctx = NULL;
	EVP_PKEY *key = NULL;

	if (!(ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!(key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED448, NULL, pub, ED448_LEN))) goto cleanup;

	if (EVP_DigestVerifyInit(ctx, NULL, NULL, NULL, key) < 1) goto cleanup;
	if (EVP_DigestVerify(ctx, sig, ED448_SIG_LEN, msg, msgLen) < 1) goto cleanup;
	ret = true;

cleanup:
	if (ctx) EVP_MD_CTX_free(ctx);
	if (key) EVP_PKEY_free(key);
	return ret;
}
