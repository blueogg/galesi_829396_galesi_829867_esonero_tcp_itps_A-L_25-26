/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void errorhandler(char* errormessage){

	printf("%s\n", errormessage);
}
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {

	// TODO: Implement server logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()");
		return 0;
	}
#endif

	int my_socket;
	my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
	errorhandler("Creazione socket fallita");
	clearwinsock();
	return -1;
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(my_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
	errorhandler("bind() failed");
	closesocket(my_socket);
	clearwinsock();
	return -1;
	}

	if(listen(my_socket, QUEUE_SIZE) < 0){

		errorhandler("listen() fallito");
		return -1;
	}

	struct sockaddr_in client_addres;
	int client_socket;
	int client_len;
	printf("%s\n", "Attendo richieste. . .");

	while (1) {
		client_len = sizeof(client_addres);
		if ((client_socket = accept(my_socket, (struct sockaddr *)&client_addres, &client_len)) < 0) {
			errorhandler("accept() failed.\n");
			closesocket(client_socket);
			continue;
		}

		puts("test");
		// TODO: Implement server logic here (recv, process, send)

		closesocket(client_socket);
	}

	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
}
