#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h> 

#include <sys/socket.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define CLIENTPORT 8080
#define BUFFSIZE 4096
#define SOCKETERROR (-1)

#define SA struct sockaddr

/* Simple TCP Client written in C to act as a "beacon" with the communication stack. Written to be minimal and executed on load of initramfs of target machine which pings back to it's designated server. */


void server_Check(int server_socket) {
	struct ifreq ifr;

	char msg[BUFFSIZE];
	char id_tag[32];
	unsigned char ipAddr[15];

	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, "enp6s18", IFNAMSIZ - 1); //interface to be configured, also to be assigned by .ini
	ioctl(server_socket, SIOCGIFADDR, &ifr);
	strcpy(ipAddr, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));

	DIR *d;
	struct dirent *dir;
	d = opendir("data/");
  	if (d) {
		while ((dir = readdir(d)) != NULL) {
    		//printf("%s\n", dir->d_name);
			if (strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0) {
				write(server_socket, dir->d_name, sizeof(dir->d_name));
				bzero(id_tag, sizeof(id_tag));
				read(server_socket, msg, sizeof(msg));
				printf("%s\n", msg);
				bzero(msg, sizeof(msg));
				write(server_socket, ipAddr, sizeof(ipAddr));
				close(server_socket);
			}
    	}
    	closedir(d);
	}


}

int main(int argv, char **argc) {
	int server_socket, connfd;
	struct sockaddr_in client_addr, cli;

	// socket create and verification
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&client_addr, sizeof(client_addr));

	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("10.0.9.121"); //target server address, soon to be passed via .ini file
	client_addr.sin_port = htons(CLIENTPORT);

	while(1) {
		if (connect(server_socket, (SA*)&client_addr, sizeof(client_addr))
			!= 0) {
			printf("connection with the server failed...\n");
			sleep(1);
			continue;
		} 
		else
			printf("connected to the server..\n");

		server_Check(server_socket);

		close(server_socket);
		break;
	}
	return 0;
}

