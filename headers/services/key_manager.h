#include "server.h"
#include "server_utils.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include "RSA.h"

EventResponse service_key_manager_callback(Context* context) {
    Cookies cookies = context_to_cookies(context);
    
    Cookie* serverPublicKey = get_cookie(cookies, "server-public-key");
    if (serverPublicKey == NULL) {
        RSA_Keyset* keyset = gen_keyset();
        StrKeyset strkeyset = keyset_to_strkeyset(keyset);
        EVP_PKEY_free(keyset);
        char* pubkey_str = malloc(sizeof(strkeyset.pubkey));
        if (strkeyset.err == 0 && pubkey_str != NULL) {
            memcpy(pubkey_str, strkeyset.pubkey + 27, strlen(strkeyset.pubkey) - 27 - 26);

            printf("pubkey raw {{\n%s}}", strkeyset.pubkey);
            
            printf("pubkey fmt {{\n");
            for (char* c = pubkey_str; *c != '\0'; c++) {
                if (*c == '\n') printf("\\n\n");
                else printf("%c", *c);
            }
            printf("}}\n");
    
            set_cookie(context, "server-public-key", pubkey_str, COOKIE_DEALLOC_TYPE_VALUE);
        }
    };

    
    Cookie* clientPublicKey = get_cookie(cookies, "client-public-key");
    if (clientPublicKey == NULL) {
        // return event_response_err(400);
    }

    

    return event_response_ok();
}
