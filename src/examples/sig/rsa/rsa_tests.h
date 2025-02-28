/*
 *  Copyright (C) 2021 - This file is part of libecc project
 *
 *  Authors:
 *      Ryad BENADJILA <ryadbenadjila@gmail.com>
 *      Arnaud EBALARD <arnaud.ebalard@ssi.gouv.fr>
 *
 *  This software is licensed under a dual BSD and GPL v2 license.
 *  See LICENSE file at the root folder of the project.
 */
#ifndef __RSA_TESTS_H__
#define __RSA_TESTS_H__

/* Test suite for RSA PKCS#1 algorithms */
#include "rsa.h"

typedef enum {
	RSA_PKCS1_v1_5_ENC = 0,
	RSA_PKCS1_v1_5_SIG = 1,
	RSA_OAEP_ENC = 2,
	RSA_PSS_SIG = 3,
} rsa_alg_type;

typedef struct {
	const char *name;
	rsa_alg_type type;
	u32 modbits;
        gen_hash_alg_type hash;
	const u8 *n;
	u16 nlen;
	const u8 *d;
	u16 dlen;
	const u8 *e;
	u16 elen;
	const u8 *p;
	u16 plen;
	const u8 *q;
	u16 qlen;
	const u8 *dP;
	u16 dPlen;
	const u8 *dQ;
	u16 dQlen;
	const u8 *qInv;
	u16 qInvlen;
	const u8 *m;
	u16 mlen;
	const u8 *res;
	u16 reslen;
	const u8 *salt;
	u16 saltlen;
} rsa_test;


ATTRIBUTE_WARN_UNUSED_RET static inline int perform_rsa_tests(const rsa_test **tests, u32 num_tests)
{
	int ret = 0, cmp;
	unsigned int i;

	for(i = 0; i < num_tests; i++){
		const rsa_test *t = tests[i];
		u32 modbits = t->modbits;
		rsa_pub_key pub;
		rsa_priv_key priv;

		/* Import the keys */
		ret = rsa_import_pub_key(&pub, t->n, (u16)t->nlen, t->e, (u16)t->elen); EG(ret, err1);
		if(t->p == NULL){
			ret = rsa_import_simple_priv_key(&priv, t->n, (u16)t->nlen, t->d, (u16)t->dlen); EG(ret, err1);
		}
		else{
			ret = rsa_import_crt_priv_key(&priv, t->p, (u16)t->plen, t->q, (u16)t->qlen, t->dP, (u16)t->dPlen, t->dQ, (u16)t->dQlen, t->qInv, (u16)t->qInvlen, NULL, NULL, 0); EG(ret, err1);
		}
		/* Perform our operation */
		switch(t->type){
			case RSA_PKCS1_v1_5_ENC:{
				u8 cipher[NN_USABLE_MAX_BYTE_LEN];
				u16 clen;
				if(t->salt != NULL){
					clen = sizeof(cipher);
					ret = rsaes_pkcs1_v1_5_encrypt(&pub, t->m, t->mlen, cipher, &clen, modbits, t->salt, t->saltlen); EG(ret, err1);
					/* Check the result */
					MUST_HAVE((clen == t->reslen), ret, err1);
					ret = are_equal(t->res, cipher, t->reslen, &cmp); EG(ret, err1);
					MUST_HAVE(cmp, ret, err1);
				}
				/* Try to decrypt */
				clen = sizeof(cipher);
				ret = rsaes_pkcs1_v1_5_decrypt(&priv, t->res, t->reslen, cipher, &clen, modbits); EG(ret, err1);
				/* Check the result */
				MUST_HAVE((clen == t->mlen), ret, err1);
				ret = are_equal(t->m, cipher, t->mlen, &cmp); EG(ret, err1);
				MUST_HAVE(cmp, ret, err1);
				break;
			}
			case RSA_OAEP_ENC:{
				u8 cipher[NN_USABLE_MAX_BYTE_LEN];
				u16 clen;
				if(t->salt != NULL){
					clen = sizeof(cipher);
					ret = rsaes_oaep_encrypt(&pub, t->m, t->mlen, cipher, &clen, modbits, NULL, 0, t->hash, t->hash, t->salt, t->saltlen); EG(ret, err1);
					/* Check the result */
					MUST_HAVE((clen == t->reslen), ret, err1);
					ret = are_equal(t->res, cipher, t->reslen, &cmp); EG(ret, err1);
					MUST_HAVE(cmp, ret, err1);
				}
				/* Try to decrypt */
				clen = sizeof(cipher);
				ret = rsaes_oaep_decrypt(&priv, t->res, t->reslen, cipher, &clen, modbits, NULL, 0, t->hash, t->hash); EG(ret, err1);
				/* Check the result */
				MUST_HAVE((clen == t->mlen), ret, err1);
				ret = are_equal(t->m, cipher, t->mlen, &cmp); EG(ret, err1);
				MUST_HAVE(cmp, ret, err1);
				break;
			}
			case RSA_PKCS1_v1_5_SIG:{
				u8 sig[NN_USABLE_MAX_BYTE_LEN];
				u16 siglen = sizeof(sig);
				ret = rsassa_pkcs1_v1_5_verify(&pub, t->m, t->mlen, t->res, t->reslen, modbits, t->hash); EG(ret, err1);
				/* Try to sign */
				ret = rsassa_pkcs1_v1_5_sign(&priv, t->m, t->mlen, sig, &siglen, modbits, t->hash); EG(ret, err1);
				/* Check the result */
				MUST_HAVE((siglen == t->reslen), ret, err1);
				ret = are_equal(t->res, sig, t->reslen, &cmp); EG(ret, err1);
				MUST_HAVE(cmp, ret, err1);
				break;
			}
			case RSA_PSS_SIG:{
				if(t->salt == NULL){
					/* In case of NULL salt, default saltlen value is the digest size */
					u8 digestsize, blocksize;
					ret = gen_hash_get_hash_sizes(t->hash, &digestsize, &blocksize); EG(ret, err1);
					ret = rsassa_pss_verify(&pub, t->m, t->mlen, t->res, t->reslen, modbits, t->hash, t->hash, digestsize); EG(ret, err1);
				}
				else{
					ret = rsassa_pss_verify(&pub, t->m, t->mlen, t->res, t->reslen, modbits, t->hash, t->hash, t->saltlen); EG(ret, err1);
				}
				if(t->salt != NULL){
					/* Try to sign */
					u8 sig[NN_USABLE_MAX_BYTE_LEN];
					u16 siglen = sizeof(sig);
					ret = rsassa_pss_sign(&priv, t->m, t->mlen, sig, &siglen, modbits, t->hash, t->hash, t->saltlen, t->salt); EG(ret, err1);
					/* Check the result */
					MUST_HAVE((siglen == t->reslen), ret, err1);
					ret = are_equal(t->res, sig, t->reslen, &cmp); EG(ret, err1);
					MUST_HAVE(cmp, ret, err1);
				}
				break;
			}
			default:{
				ret = -1;
				break;
			}
		}
err1:
		if(ret){
			ext_printf("[-] Test %s failed (modbits = %d)\n", t->name, t->modbits);
			goto err;
		}
		else{
			ext_printf("[+] Test %s passed (modbits = %d)\n", t->name, t->modbits);
		}
	}

	if(!ret){
		ext_printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\t=== [+] All RSA tests went OK! ===\n");
	}
err:
	return ret;
}

#endif /* __RSA_TESTS_H__ */
