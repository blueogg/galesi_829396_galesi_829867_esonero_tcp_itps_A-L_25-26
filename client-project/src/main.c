/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */
#define MAXREQLENGHT 64
#define MAXADDRLENGHT 16
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

void printarray(const char a[]){
int i = 0;
	while(a[i] != '\0'){
		if(i == 0) { printf(". %c", toupper(a[i])); i++;
		}else {
		printf("%c", tolower(a[i]));
		i++;
		}
	}
}

weather_request_t* requestCreate(const char richiesta[]){
	weather_request_t* req = (weather_request_t*)malloc(sizeof(weather_request_t));

	req->type = richiesta[0];

	int i  = 2;
	int j = 0;
	char nome[64] = {};
	while(richiesta[i] != '\0'){
		nome[j] = richiesta[i];
		i++;
		j++;
	}
	strcpy(req->city, nome);

	return req;
}

size_t getreqsize(void){
	return sizeof(weather_request_t);
}

size_t getressize(void){
	return sizeof(weather_response_t);
}


int main(int argc, char *argv[]) {

	char* richiesta = NULL;
	long port = 0;
	char* indirizzo = NULL;
	int check = 0;
	int counterR = 0;

	for(int i = 1; i < argc; i++){


		if((argv[i][0] == '-') && (argv[i][1] != 'r' && argv[i][1] != 's' && argv[i][1] != 'p')){

			errorhandler("Flag non gestito");
			errorhandler("client-project [-s server] [-p port] -r 'type city'");
			return -1;
		}


		if(strcmp(argv[i], "-s") == 0){
			check++;
			indirizzo = malloc(MAXADDRLENGHT * sizeof(char));
			strcpy(indirizzo, argv[i + 1]);

		}
		if(strcmp(argv[i], "-p") == 0){
		port = strtol(argv[i + 1], NULL, 10);

		}

		if(strcmp(argv[i], "-r") == 0){
			counterR++;
			richiesta = malloc(MAXREQLENGHT * sizeof(char));
			strcpy(richiesta, argv[i  + 1]);
		}

	}


	if(port < 0 || port > 65535) errorhandler("Porta non valida");
	if(check != 0){
		check = inet_addr(indirizzo);
		if(check == INADDR_NONE){
		errorhandler("Indirizzo in formato non valido");
		return -1;
		}
	}
	if(counterR == 0) { errorhandler("Il parametro -r e' obbligatorio"); return -1;}

	char _richiesta[MAXREQLENGHT];
	strcpy(_richiesta, richiesta);



#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Errore su WSAStartup()");
		return 0;
	}
#endif

	int my_socket;
	my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(my_socket < 0) {errorhandler("Creaziones socket fallita"); clearwinsock(); return -1;}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;

    const char *ip_to_use = (indirizzo == NULL) ? "127.0.0.1" : indirizzo;
    server_addr.sin_addr.s_addr = inet_addr(ip_to_use);

	server_addr.sin_port = port == 0 ? htons(SERVER_PORT) : htons(port);

	if(connect(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		errorhandler("connect() fallito");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	weather_request_t* req = requestCreate(_richiesta);
    weather_response_t res;

	send(my_socket, req, getreqsize(), 0);
	recv(my_socket, &res, getressize(), 0);
	printf("Ricevuto risultato dal server ip %s", inet_ntoa(server_addr.sin_addr));
	if(res.status == 0){

		int i  = 2;
		int j = 0;
		char nome[64];
		while(richiesta[i] != '\0'){
			nome[j] = richiesta[i];
			i++;
			j++;
		}

		switch(res.type){

		case 't':
			printarray(nome);
			printf(": Temperatura = %.1f°C\n", res.value);
				break;

		case 'h':
			printarray(nome);
			printf(": Umidità = %.1f%c\n", res.value, 37);

			break;
		case 'w':
			printarray(nome);
			printf(": Vento = %.1fkm/h\n", res.value);

			break;
		case 'p':
			printarray(nome);
			printf(": Pressione = %.1fhPa\n", res.value);

			break;
		}
		}
		if(res.status == 1){

			errorhandler(". Città non disponibile");
				return -1;
		}

		if(res.status == 2){
			errorhandler(". Richiesta non valida");

		}

		closesocket(my_socket);
		clearwinsock();
		return 0;


	}







