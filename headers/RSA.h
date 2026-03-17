#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <stddef.h>


#define RSA_PADDING RSA_PKCS1_OAEP_PADDING

typedef EVP_PKEY RSA_Keyset;
typedef EVP_PKEY RSA_Pubkey;
typedef EVP_PKEY RSA_Privkey;

typedef struct {
    char pubkey[1024];
    char privkey[4096];
    int err;
} StrKeyset;


RSA_Keyset* gen_keyset();
unsigned char* encrypt_buffer(
    RSA_Pubkey* pubkey,
    const unsigned char* msg, size_t msg_len,
    size_t* out_len
);
unsigned char* decrypt_buffer(
    RSA_Privkey* privkey,
    const unsigned char* encrypted_msg, size_t encrypted_msg_len,
    size_t *decrypted_msg_len
);
StrKeyset keyset_to_strkeyset(RSA_Keyset* keyset);
RSA_Pubkey* load_pubkey_from_str(const char* pubkey_str);
RSA_Privkey* load_privkey_from_str(const char* privkey_str);

