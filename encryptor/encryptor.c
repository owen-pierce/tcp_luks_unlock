#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "aes.h"
#include <sodium.h>

/*  The "Encryptor" application, is a WIP used for encrypting a given password which is currently stored in the "data" directory within the source folder.
    To generate an encrypted password simply pass encryptor 2 arguments: path, code
    Example: ./encryptor ../data/ myPassword
    Encryptor will return an encrypted version of the passed password in byte data built to be passed over SSH.
*/

int main(int argc, const char **argv) {

    FILE *fp;
    uint32_t intCode;
    char cmpInt[32];
    char *path = malloc(strlen(argv[1])+1);
    strcpy(path,argv[1]);


    if (sodium_init() < 0) {
        exit(1);
    }

    struct AES_ctx ctx;
    uint8_t key[16];
    uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t str[32];

    if (argc != 3) {
        printf("Inavlid number of arguments passed.\n");
        return 0;
    } else {
        while(1) {
            do {
                intCode = randombytes_uniform(99999999);
                printf("%d\n", intCode);
            } while (intCode < 10000000);
            sprintf(cmpInt, "%u", intCode);
            strcat(path, cmpInt);
            if (access(path, F_OK) == 0) {
                continue;
            } else {
                sprintf((char*)key, "%d", intCode);
                strncpy((char*)str, argv[2], 32);
                AES_init_ctx_iv(&ctx, key, iv);
                AES_CBC_encrypt_buffer(&ctx, str, 32);
                fp = fopen(path, "w");
                fwrite(&str, sizeof(unsigned char), 32, fp);
                fclose(fp);
                break;
            }
        }
        free(path);
        return 0;
    }
}
