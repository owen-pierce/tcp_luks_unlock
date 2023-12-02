#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "aes.h"
#include <sodium.h>

int main(int argc, const char **argv) {

    FILE *fp;
    uint32_t myInt;
    char cmpInt[32];
    char path[32]="/home/owen/server_code/data/";

    if (sodium_init() < 0) {
        exit(1);
    }

    struct AES_ctx ctx;
    uint8_t key[16];
    uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t str[32];

    if (argc != 2) {
        printf("Inavlid number of arguments passed.\n");
        return 0;
    } else {
        while(1) {
            do {
                myInt = randombytes_uniform(99999999);
                printf("%d\n", myInt);
            } while (myInt < 10000000);
            sprintf(cmpInt, "%u", myInt);
            strcat(path, cmpInt);
            if (access(path, F_OK) == 0) {
                continue;
            } else {
                sprintf((char*)key, "%d", myInt);
                strncpy((char*)str, argv[1], 32);
                AES_init_ctx_iv(&ctx, key, iv);
                AES_CBC_encrypt_buffer(&ctx, str, 32);
                fp = fopen(path, "w");
                fwrite(&str, sizeof(unsigned char), 32, fp);
                fclose(fp);
                break;
            }
        }
        return 0;
    }
}
