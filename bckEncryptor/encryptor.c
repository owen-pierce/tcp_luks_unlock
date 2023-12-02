#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "aes.h"

int main(int argc, const char **argv) {

    FILE *fp;

    struct AES_ctx ctx;
    uint8_t key[16];
    uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t str[32];

    if (argc != 3) {
        printf("Inavlid number of arguments passed.\n");
        return 0;
    } else {
        strncpy((char*)key, argv[1], 16);
        strncpy((char*)str, argv[2], 32);
        AES_init_ctx_iv(&ctx, key, iv);
        AES_CBC_encrypt_buffer(&ctx, str, 32);

        fp = fopen("/home/owen/server_code/bufferOut", "w");
        fwrite(&str, sizeof(unsigned char), 32, fp);
        fclose(fp);
        return 0;
    }
}
