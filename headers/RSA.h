#pragma once
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <stddef.h>


#define RSA_PADDING RSA_PKCS1_OAEP_PADDING



#define PUBKEY_STR "-----BEGIN PUBLIC KEY-----\n\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA6uuKjV+aCxqB1wDAg51E\n\
sBsC4HgYaw2Wh0rLvQ7bsJwDnOhP5DiCgJLhFX+Vubq5qrNQDL7+qoovwPsymc8J\n\
U4P188vC+KWYzCH8dJwtmcMNOqID5jPL7B5f9GqULTkHlpeh4rist89KpEKtXPpS\n\
nhbR2eu4nC5VX/SGzHagdQX0ASu/JMsOANCNTwyB9se0MJAut1IRouB2qXtKf/N9\n\
Eqw/yolH2Vs+l4tqA/ElHmRQSz7B8qmBK3zifaM2Eoar3KhouJhGh3ToB4tEs6iQ\n\
ZQlQ4HceONxLfYoOwbYMTPMmP80nF1KhYtihsbMr5gT9lAPGsTj0zQuPtwmHJycE\n\
3QIDAQAB\n\
-----END PUBLIC KEY-----\
"


#define PRIVKEY_STR "-----BEGIN PRIVATE KEY-----\n"\
"MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDq64qNX5oLGoHX\n"\
"AMCDnUSwGwLgeBhrDZaHSsu9DtuwnAOc6E/kOIKAkuEVf5W5urmqs1AMvv6qii/A\n"\
"+zKZzwlTg/Xzy8L4pZjMIfx0nC2Zww06ogPmM8vsHl/0apQtOQeWl6HiuKy3z0qk\n"\
"Qq1c+lKeFtHZ67icLlVf9IbMdqB1BfQBK78kyw4A0I1PDIH2x7QwkC63UhGi4Hap\n"\
"e0p/830SrD/KiUfZWz6Xi2oD8SUeZFBLPsHyqYErfOJ9ozYShqvcqGi4mEaHdOgH\n"\
"i0SzqJBlCVDgdx443Et9ig7BtgxM8yY/zScXUqFi2KGxsyvmBP2UA8axOPTNC4+3\n"\
"CYcnJwTdAgMBAAECggEAJDhRwe9aCFk7aKNoi0Jdu0tG28YYebT8Hvr0w779N1Mu\n"\
"qWwWGUlvMZSz2IU12A+Gnd+MuZJfIHsy1q3ERPurY/GGI+MoOf8KOnsI+g+0Zi8x\n"\
"JHejY0RcLhkW0U/3bauL1HenkZXIp4Rx7TaMlTfRMiexjkhuS8JHDobcm7y0i6Ym\n"\
"I2yAL2vRMdZQHK5nIc3SVFF3ThLDuVfslnWt4ACbUzEsmlnF+E21KLSsZCBAF+zQ\n"\
"Iy7fUXbIoDpSLqso7T7StdesrSw4HXl0NGHJMtKZeTy7l9BYxeTs0nRrE524Tx9R\n"\
"F9qECuqD0UZxMItTX2Yv6iWLaw15Mk0Ufqyq6wqlwQKBgQD/sAWmHZkM8VCCS6gL\n"\
"y1JwesZthTOTT4/+Iaqv9w3HoST8llzokcC7iSp+D3dte4T6y9XzcLFHQ3KyiG3+\n"\
"gvOOoDq7CserRI4zxrPQi7MehqVljgDV2Yu9LFjPkWWQkDsi6hZzfWP5Lbsltva6\n"\
"N3xWgH+oIrnKbUZ1d0M0U8YwPQKBgQDrNQXukXZeTZJCpkw1NHdGd9YASs56KvBZ\n"\
"w4kzUHnmJJXSo3/ht12RpsY8Iin7U9qodUMrzxMq/w7m52d3kp8JGl48cmtFWDgT\n"\
"jlRntrcgELnpTole4GfIafbSrU+cdi6qSi7x/baywJRBvyjug/gU7x9/sIMWD5oq\n"\
"Gj/EBK3RIQKBgQDj6dF/O07cHJa5LhtDOB/qJ2aM5XkNzJP74aK2a7AazGedx6JV\n"\
"pOZ5xFDeQYchTkNBE0VJVKCRd2vb/ISfo7ba4bS4WWmWHuvq9yn9sSkfAsKX/bZ8\n"\
"6QgIvmkDzQUNkaNFAgi1NYZR7ROvEKJpZ2Ar8yrXguy9ph5yQrbjA/HbuQKBgQCa\n"\
"2BBsq7TahoT4fdSD3mN7r97h2F2wTd5nqfQEJUHrAkDkKCFDbt0Zy0qMQg2TWPQF\n"\
"nRU4nR3IW5vq5YLXVF6Atqw9bTIzt81LCpOEp+RNJtXNw5dqL6ADb8c/kTttDNzR\n"\
"3P6k71u8uE2iLv/6bkT/OyFohcL23hSnDRrXqCDqIQKBgQCeMbAEu2RKBCJBFfjB\n"\
"PM4ywoQUbVtFm5wzDSNyfJHynql3e/4I5aBIksM/RLI5zHBWq6EN7qtCjrZranlj\n"\
"1u0BgqBgF34Snz6ZKVh3Kw/Agwak8W8FRnRljAHdzP7EjPGzIWUJxTM2WM3EY1vY\n"\
"hp8ZaJECmU7cQrgv5YHTkz7f+w==\n"\
"-----END PRIVATE KEY-----"





typedef EVP_PKEY RSA_Keyset;
typedef EVP_PKEY RSA_Pubkey;
typedef EVP_PKEY RSA_Privkey;

static RSA_Pubkey* server_pubkey = NULL;
static RSA_Privkey* server_privkey = NULL;

typedef struct {
    char pubkey[1024];
    char privkey[4096];
    int err;
} StrKeyset;

typedef struct {
    RSA_Pubkey* pub;
    RSA_Privkey* priv;
} Keyset;


Keyset gen_keyset();
unsigned char* encrypt_buffer(
    RSA_Pubkey* pubkey,
    const unsigned char* msg, size_t msg_len,
    size_t* out_len
);
unsigned char* decrypt_buffer(
    RSA_Privkey* privkey,
    unsigned char* encrypted_msg, size_t encrypted_msg_len,
    size_t *decrypted_msg_len
);
StrKeyset keyset_to_strkeyset(RSA_Keyset* keyset);
RSA_Pubkey* load_pubkey_from_str(const char* pubkey_str);
RSA_Privkey* load_privkey_from_str(const char* privkey_str);

