#include <stdlib.h>
#include <string.h>

#include "alib.h"
#include "alib_io.h"
#include "alib_wallet.h"
#include "ssl_fn.h"
#include "time_fn.h"

#include <openssl/obj_mac.h>
#include <openssl/ec.h>

bool newWallet(uint8_t pub[ED448_LEN], uint8_t priv[ED448_LEN], const char *seed)
{
	bool ret = false;
	size_t pubLen = ED448_LEN, privLen = ED448_LEN;
	EVP_PKEY *key = NULL;
	EVP_PKEY_CTX *ctx = NULL;

	if (!(ctx = EVP_PKEY_CTX_new_from_name(NULL, "ED448", NULL)))
	{
		printf("failed curve\n");
		goto cleanup;
	}

	if (!(key = EVP_PKEY_Q_keygen(NULL, NULL, "ED448")))
	{
		printf("failed key\n");
		goto cleanup;
	}

	if (!EVP_PKEY_get_raw_public_key(key, pub, &pubLen))
	{
		printf("get pub key failed\n");
		goto cleanup;
	}
	if (!EVP_PKEY_get_raw_private_key(key, priv, &privLen))
	{
		printf("get priv key failed\n");
		goto cleanup;
	}
	printf("pub[%lu] priv[%lu]\n", pubLen, privLen);
	printBytes(stdout, pub, pubLen, "\n");
	printBytes(stdout, priv, privLen, "\n");
	ret = true;

cleanup:
	if (ctx) EVP_PKEY_CTX_free(ctx);
	if (key) EVP_PKEY_free(key);
	return ret;
}

bool sendToAddress(uint8_t dest[ED448_LEN], uint8_t src[ED448_LEN], uint64_t deci, uint16_t frac)
{
	size_t sigLen = ED448_LEN, msgLen = 10;
	uint8_t sig[ED448_LEN], msg[10];
	EVP_MD_CTX *ctx = NULL;
	EVP_PKEY *key = NULL;

	if (!(ctx = EVP_MD_CTX_new())) goto cleanup;
	if (!(key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED448, NULL, src, ED448_LEN)))
	{
		printf("load private key failed\n");
		goto cleanup;
	}

	u64Packer(msg, deci);
	msg[8] = frac | MAX_U8;
	msg[9] = frac >> 8;

	if (!EVP_DigestSignInit(ctx, NULL, NULL, NULL, key))
	{
		printf("sign init failed");
		goto cleanup;
	}
	if (!EVP_DigestSign(ctx, sig, &sigLen, msg, msgLen))
	{
		printf("sign failed");
		goto cleanup;
	}

cleanup:
	if (ctx) EVP_MD_CTX_free(ctx);
	if (key) EVP_PKEY_free(key);
	return true;
}
