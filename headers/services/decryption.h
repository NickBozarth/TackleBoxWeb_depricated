#include "server.h"
#include "RSA.h"
#include <stddef.h>
#include <stdio.h>

EventResponse decryprion_callback(Context* context) {
    HTTPResponse* client_resp = (HTTPResponse*)context->items[0].data;
    printf("items[0] = %s\n", context->items[0].name);
    Keyset keys = gen_keyset();
    printf("SERVER_PRIVKEY = %p\n", keys.priv);


    printf("RESPONSE {{");
    for (int i = 0; i < client_resp->body_len; i++) {
        if (i % 8 == 0) printf("\n\t");
        else if (i % 4 == 0) printf(" ");

        char body_char = client_resp->body[i];
        printf("%3hhu, ", body_char);
    }
    printf("\n}}\n");


    size_t decrypted_msg_len;
    unsigned char* decrypted_msg = decrypt_buffer(keys.priv, (unsigned char*)client_resp->body, client_resp->body_len, &decrypted_msg_len);
    if (decrypted_msg < 10) printf("NOOOOOOOO %li\n", (long)decrypted_msg);
    else printf("DECRYPTED BUFFER {{\n%s\n}}\n", decrypted_msg);

    // printf("{{{\n");
    // if (decrypted_msg < (unsigned char*)10) printf("failed to decrypt buffer %lu\n", (size_t)decrypted_msg);
    // else {
    //     for (int i = 0; i < decrypted_msg_len; i++) {
    //         printf("%c", decrypted_msg[i]);
    //     }
    // }
    // printf("}}}\n");
    return event_response_err(400);
}
