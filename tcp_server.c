#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "myqueue.h"
#include "aes.h"
#include "ini.h"
#include "ezini.h"

#define SERVERPORT 8080
#define BUFFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;


/* A simple multi-threaded TCP Server created to interact with LUKS encrypted clients and decrypt them via SSH securely. Current WIP and needs crypto method updated for communicating keys over the network */


typedef struct {
	const char* DATA_STORE_PATH;
    const char* SHOW_DEBUG_INFO;
    const char* SSH_PORT_NUM;
    const char* SSH_IDENTITY_FILE;
} iniConfig;

static int iniHandler(void* Options, const char* section, const char* name, const char* value) {
	iniConfig* pIniConfig = (iniConfig*)Options;
	#define match_Int(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
		
	if (match_Int("Options", "DATA_STORE_PATH")) {
		pIniConfig->DATA_STORE_PATH = strdup(value);
	} else if (match_Int("Options", "SHOW_DEBUG_INFO")) {
		pIniConfig->SHOW_DEBUG_INFO = strdup(value);
	} else if (match_Int("Options", "SSH_PORT_NUM")) {
		pIniConfig->SSH_PORT_NUM = strdup(value);
	} else if (match_Int("Options", "SSH_IDENTITY_FILE")) {
		pIniConfig->SSH_IDENTITY_FILE = strdup(value);
	}
	else {
	    return 0;  /* unknown section/name, error */
	}
	return 1;
}

bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

void * handle_connection(void* p_client_socket);

int check(int exp, const char *msg);

void * thread_function(void *arg);

int main(int argc, char **argv) {

	int server_socket, client_socket, addr_size;
	SA_IN server_addr, client_addr;

	ini_entry_list_t list;

	char *DATA_STORE_PATH_INI = malloc(256);
    char *SSH_IDENTITY_FILE_INI = malloc(256);
	char *SHOW_DEBUG_INFO_INI = malloc(256);
    char *SSH_PORT_NUM_INI = malloc(256);

    strcpy(DATA_STORE_PATH_INI, "data/");
	strcpy(SSH_IDENTITY_FILE_INI, ".ssh/id_rsa");
    strcpy(SHOW_DEBUG_INFO_INI, "0");
    strcpy(SSH_PORT_NUM_INI, "2222");

	if (access("settings.ini", F_OK) != 0) {

       	list = NULL;
   		AddEntryToList(&list, "Options", "DATA_STORE_PATH", DATA_STORE_PATH_INI);
		AddEntryToList(&list, "Options", "SSH_IDENTITY_FILE", SSH_IDENTITY_FILE_INI);
   		AddEntryToList(&list, "Options", "SHOW_DEBUG_INFO", SHOW_DEBUG_INFO_INI);
   		AddEntryToList(&list, "Options", "SSH_PORT_NUM", SSH_PORT_NUM_INI);

   		if (0 != MakeINIFile("settings.ini", list)) {
        	printf("Error making settings.ini\n");
    	}
    	FreeList(list);

		iniConfig int_Read;
		if (ini_parse("settings.ini", iniHandler, &int_Read) < 0) {
			printf("Can't load settings.ini.\n");
			return 1;
		}

	strncpy(DATA_STORE_PATH_INI, int_Read.DATA_STORE_PATH, 32);
	strncpy(SSH_IDENTITY_FILE_INI, int_Read.SSH_IDENTITY_FILE, 32);
	strncpy(SHOW_DEBUG_INFO_INI, int_Read.SHOW_DEBUG_INFO, 3);
	strncpy(SSH_PORT_NUM_INI, int_Read.SSH_PORT_NUM, 5);

	printf("Generated settings.ini you must update file before server use.\n");
	printf("Data Store Path: %s\n", DATA_STORE_PATH_INI);
	printf("SSH Identity File Path: %s\n", SSH_IDENTITY_FILE_INI);
	printf("Show Debug Info: %s\n", SHOW_DEBUG_INFO_INI);
	printf("SSH Port Number: %s\n", SSH_PORT_NUM_INI);
	return 0;

}

	for (int i=0; i < THREAD_POOL_SIZE; i++) {
		pthread_create(&thread_pool[i], NULL, thread_function, NULL);
	}

	check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "FAILED TO CREATE SOCKET");
	check((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0), "FAILED TO SET SOCKET OPTIONS");

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVERPORT);

	check(bind(server_socket,(SA*)&server_addr, sizeof(server_addr)), "BIND FAILED");
	check(listen(server_socket, SERVER_BACKLOG), "LISTEN FAILED");

	while(true) {
	
		printf("Waiting for connections...\n");
		addr_size = sizeof(SA_IN);
		check(client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size), "ACCEPT FAILED");
		//printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));
		// char addr_space[INET_ADDRSTRLEN];
		// inet_ntop( AF_INET, &client_addr.sin_addr, addr_space, sizeof(addr_space) );


		printf("Connection Established...\n");
		int *pclient = malloc(sizeof(int));
		*pclient = client_socket;
		pthread_mutex_lock(&mutex);
		enqueue(pclient);
		pthread_cond_signal(&condition_var);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}

int check(int exp, const char *msg) {
	if (exp == SOCKETERROR) {
		perror(msg);
		exit(1);
	}
	return exp;
}

void * thread_function(void *arg) {
	while (true) {
		int *pclient;
		pthread_mutex_lock(&mutex);
		if ((pclient = dequeue()) == NULL) {
			pthread_cond_wait(&condition_var, &mutex);
			pclient = dequeue();
		}
		pthread_mutex_unlock(&mutex);

		if (pclient != NULL) {
			handle_connection(pclient);
		}
	}
}


void * handle_connection(void* p_client_socket) {

	int client_socket = *((int*)p_client_socket);
	free(p_client_socket);
	char buffer[BUFFSIZE];
	char cli_addr[BUFFSIZE];
	char reply[BUFFSIZE]="success";
	int msgsize = 0;

	ini_entry_list_t list;

	char *DATA_STORE_PATH_INI = malloc(256);
    char *SSH_IDENTITY_FILE_INI = malloc(256);
	char *SHOW_DEBUG_INFO_INI = malloc(256);
    char *SSH_PORT_NUM_INI = malloc(256);

    // strcpy(DATA_STORE_PATH_INI, "data/");
	// strcpy(SSH_IDENTITY_FILE_INI, ".ssh/id_rsa");
    // strcpy(SHOW_DEBUG_INFO_INI, "0");
    // strcpy(SSH_PORT_NUM_INI, "2222");

	if (access("settings.ini", F_OK) == 0) {

		iniConfig int_Read;
		if (ini_parse("settings.ini", iniHandler, &int_Read) < 0) {
				printf("Can't load.\n");
				return NULL;
			}

			strncpy(DATA_STORE_PATH_INI, int_Read.DATA_STORE_PATH, 32);
			strncpy(SSH_IDENTITY_FILE_INI, int_Read.SSH_IDENTITY_FILE, 32);
			strncpy(SHOW_DEBUG_INFO_INI, int_Read.SHOW_DEBUG_INFO, 3);
			strncpy(SSH_PORT_NUM_INI, int_Read.SSH_PORT_NUM, 5);
			
			free((void*)int_Read.DATA_STORE_PATH);
			free((void*)int_Read.SSH_IDENTITY_FILE);
			free((void*)int_Read.SHOW_DEBUG_INFO);
			free((void*)int_Read.SSH_PORT_NUM);

		} else {

			printf("Unable to load settings.ini\n");
			exit(1);

	}

	bzero(buffer, BUFFSIZE);
	bzero(cli_addr, BUFFSIZE);
	read(client_socket, buffer, sizeof(buffer));
	printf("From client: %s\t\n", buffer);
	write(client_socket, reply, sizeof(reply));
	read(client_socket, cli_addr, sizeof(cli_addr));
	printf("From client: %s\t\n", cli_addr);
	close(client_socket);

	struct AES_ctx ctx;
	uint8_t key[16];
	strncpy((char*)key,buffer,sizeof(key));
		
	uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	uint8_t str[32];

	strcat(DATA_STORE_PATH_INI, buffer);
	printf("%s\n", DATA_STORE_PATH_INI);

    char address[100] = {'\0'};
    sprintf(address, "%s", DATA_STORE_PATH_INI);

	FILE *file;
	file = fopen(address, "r");
	if (file == NULL) return NULL;
	fseek(file, 0, SEEK_END);
	long int size = ftell(file);
	fclose(file);

	file = fopen(address, "r");
	int bytes_read = fread(str, sizeof(unsigned char), size, file);
	fclose(file);
	
	if (bytes_read != 32 || isValidIpAddress(cli_addr) != true) {
		printf("Invalid communication.\n");
		return NULL;
	} else {
		
		AES_init_ctx_iv(&ctx, key, iv);
		AES_CBC_decrypt_buffer(&ctx, str, 32);

		char cmd[256];
		sprintf(cmd, "echo '%s' | ssh -tt -i %s -p %s root@%s", str, SSH_IDENTITY_FILE_INI, SSH_PORT_NUM_INI, cli_addr);
		//printf("%s\n",cmd);
		int pid = fork();
		if (pid == -1) {
			return NULL;
		}
		if (pid == 0) {
			close(1);
			
			//execlp("/bin/sh", "/bin/sh", "-c", cmd, NULL);
		} else {
			wait(NULL);
			printf("Finished!\n");
		}
	}
	
	bzero(buffer, BUFFSIZE);
	return 0;
}