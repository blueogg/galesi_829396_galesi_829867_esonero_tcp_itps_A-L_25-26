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
#include <ctype.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

size_t getreqsize(){
	return sizeof(weather_request_t);
}

size_t getressize(){
	return sizeof(weather_request_t);
}

#include <stdlib.h>

float get_temperature(void) {
    float range_min = -10.0f;
    float range_size = 50.0f;
    return ( (float)rand() / RAND_MAX ) * range_size + range_min;
}

float get_humidity(void) {
    float range_min = 20.0f;
    float range_size = 80.0f;
    return ( (float)rand() / RAND_MAX ) * range_size + range_min;
}

float get_wind(void) {
    float range_size = 100.0f;
    return ( (float)rand() / RAND_MAX ) * range_size;
}

float get_pressure(void) {
    float range_min = 950.0f;
    float range_size = 100.0f;
    return ( (float)rand() / RAND_MAX ) * range_size + range_min;
}


weather_response_t processReq(weather_request_t* req, struct sockaddr_in client){

	weather_response_t res;
    res.status = 0;
    res.type = '\0';
    res.value = 0.0f;

	printf("%s '%c ", "Richiesta", req->type);
	int i = 0;
	char nome[64] = {};
	while( req->city[i] != '\0'){
		nome[i] = req->city[i];
		nome[i] = tolower(nome[i]);
		printf("%c", req->city[i] );
		i++;

	}

	printf("' dal client ip %s\n", inet_ntoa(client.sin_addr));

	int status = 0;
	if(req->type != 't' && req->type != 'h' && req->type != 'w' && req->type != 'p'){

		status = 2;
		res.status = status;
		res.type = req->type;
		return res;

	}
	const char* lista[] = { "bari", "roma", "milano", "napoli", "torino", "palermo", "genova", "bologna", "firenze", "venezia"};
	int _check;
	int check = 0;
	for(int i = 0; i < 10; i++){
		_check = strcmp(nome, lista[i]);
		if(_check == 0){
			check++;
			break;
		}

	}

	if(check == 0){

		status = 1;
		res.status = status;
		res.type = req->type;
		return res;

	}

    res.type = req->type;

    switch(req->type) {
        case 't': res.value = get_temperature(); break;
        case 'h': res.value = get_humidity(); break;
        case 'w': res.value = get_wind(); break;
        case 'p': res.value = get_pressure(); break;
    }

	return res;
}

int main(int argc, char *argv[]) {

    srand(time(NULL));
    int port = 0;
    for(int i = 1; i < argc; i++){

		if((argv[i][0] == '-') && argv[i][1] != 'p'){
			errorhandler("Flag non gestito");
			errorhandler("server-project [-p port]");
			return -1;
		}

		if(strcmp(argv[i], "-p") == 0){
		port = strtol(argv[i + 1], NULL, 10);
		}
    }


#if defined WIN32
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
	server_addr.sin_port = port == 0 ? htons(SERVER_PORT) : htons(port);
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
	while (1) {
		client_len = sizeof(client_addres);
		if ((client_socket = accept(my_socket, (struct sockaddr *)&client_addres, &client_len)) < 0) {
			errorhandler("accept() failed.\n");
			closesocket(client_socket);
			continue;
		}

		weather_request_t req;
        weather_response_t res;
        recv(client_socket, (void*)&req, getreqsize(), 0);
		res = processReq(&req, client_addres);
        send(client_socket, (const void*)&res, getressize(), 0);

		closesocket(client_socket);
	}

	closesocket(my_socket);
	clearwinsock();
	return 0;
}
