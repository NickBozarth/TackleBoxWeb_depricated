#include "RSA.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string.h>

RSA_Keyset* gen_keyset() {
    EVP_PKEY* keyset = NULL;
    EVP_PKEY_CTX* context = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (context == NULL) return NULL;
    if (EVP_PKEY_keygen_init(context) <= 0) return NULL;
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(context, 2048) <= 0) return NULL;
    if (EVP_PKEY_keygen(context, &keyset) <= 0) return NULL;
    EVP_PKEY_CTX_free(context);
    return keyset;

}

unsigned char* encrypt_buffer(
    RSA_Pubkey* pubkey,
    const unsigned char* decrypted_msg, size_t decrypted_msg_len,
    size_t* encrypted_msg_len
) {
    EVP_PKEY_CTX* context = EVP_PKEY_CTX_new(pubkey, NULL);
    if (context == NULL) return NULL;
    if (EVP_PKEY_encrypt_init(context) <= 0) return NULL;
    if (EVP_PKEY_CTX_set_rsa_padding(context, RSA_PADDING) <= 0) return NULL;
    if (EVP_PKEY_encrypt(context, NULL, encrypted_msg_len, decrypted_msg, decrypted_msg_len) <= 0) return NULL;

    unsigned char* encrypted_msg = (unsigned char*)malloc(*encrypted_msg_len);
    if (EVP_PKEY_encrypt(context, encrypted_msg, encrypted_msg_len, decrypted_msg, decrypted_msg_len) <= 0) {
        free(encrypted_msg);
        encrypted_msg = NULL;
    }
    EVP_PKEY_CTX_free(context);
    return encrypted_msg;
}

unsigned char* decrypt_buffer(
    RSA_Privkey* privkey,
    const unsigned char* encrypted_msg, size_t encrypted_msg_len,
    size_t *decrypted_msg_len
) {
    EVP_PKEY_CTX* context = EVP_PKEY_CTX_new(privkey, NULL);
    if (context == NULL) return NULL;
    if (EVP_PKEY_decrypt_init(context) <= 0) return NULL;
    if (EVP_PKEY_CTX_set_rsa_padding(context, RSA_PADDING) <= 0) return NULL;
    if (EVP_PKEY_decrypt(context, NULL, decrypted_msg_len, encrypted_msg, encrypted_msg_len) <= 0) return NULL;

    unsigned char* decrypted_msg = malloc(*decrypted_msg_len + 1);
    if (EVP_PKEY_decrypt(context, decrypted_msg, decrypted_msg_len, encrypted_msg, encrypted_msg_len) <= 0) {
        free(decrypted_msg);
        decrypted_msg = NULL;
    } else {
        decrypted_msg[*decrypted_msg_len] = '\0';
    }
    EVP_PKEY_CTX_free(context);
    return decrypted_msg;
}

StrKeyset keyset_to_strkeyset(RSA_Keyset *keyset) {
    StrKeyset ret = { };
    BIO* pub_bio  = BIO_new(BIO_s_mem());
    BIO* priv_bio = BIO_new(BIO_s_mem());

    if (PEM_write_bio_PUBKEY(pub_bio, keyset) > 0) {
        char* data;
        long len = BIO_get_mem_data(pub_bio, &data);
        memcpy(ret.pubkey, data, len);
        ret.pubkey[len] = '\0';
    } else {
        ret.err = 1;
    }

    if (
        ret.err == 0 &&
        PEM_write_bio_PrivateKey(priv_bio, keyset, NULL, NULL, 0, NULL, NULL) > 0
    ) {
        char* data;
        long len = BIO_get_mem_data(priv_bio, &data);
        memcpy(ret.privkey, data, len);
        ret.privkey[len] = '\0';
    } else {
        ret.err = 2;
    }

    BIO_free(pub_bio);
    BIO_free(priv_bio);
    return ret;
}

EVP_PKEY* load_key_from_str(const char* pkey_str, int public) {
    BIO* bio = BIO_new_mem_buf(pkey_str, -1);
    EVP_PKEY* pkey = public ? 
        PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL) : 
        PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return pkey;
}

RSA_Pubkey* load_pubkey_from_str(const char* pubkey_str) {
    return load_key_from_str(pubkey_str, 1);
}

RSA_Privkey* load_privkey_from_str(const char* privkey_str) {
    return load_key_from_str(privkey_str, 0);
}
