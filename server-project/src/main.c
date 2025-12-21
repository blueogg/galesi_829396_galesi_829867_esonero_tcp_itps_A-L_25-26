/*
 * main.c
 *
 * UDP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a UDP server
 * portable across Windows, Linux, and macOS.
 */


#if defined WIN32
#include <winsock.h>
#include <stdint.h>
#else
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
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

int main(int argc, char *argv[]) {

    srand((unsigned int)time(NULL));
    int port = SERVER_PORT;

    for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-p") == 0 && i + 1 < argc){
		    port = strtol(argv[i + 1], NULL, 10);
            i++;
		} else {
            printf("Usage: %s [-p port]\n", argv[0]);
            return -1;
        }
    }

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket;
	my_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (my_socket < 0) {
	    errorhandler("Creazione socket fallita");
	    clearwinsock();
	    return -1;
	}

	struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(my_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
	    errorhandler("bind() failed");
	    closesocket(my_socket);
	    clearwinsock();
	    return -1;
	}

    printf("Server UDP in ascolto sulla porta %d\n", port);

	struct sockaddr_in client_addr;
	int lungh_client;
    char buffer_rx[BUFFER_SIZE];

	while (1) {
		lungh_client = sizeof(client_addr);
        memset(buffer_rx, 0, BUFFER_SIZE);

		int bytes_ricevuti = recvfrom(my_socket, buffer_rx, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &lungh_client);
        if (bytes_ricevuti < 0) {
			errorhandler("recvfrom() failed.\n");
			continue;
		}

		weather_request_t req;
        req.type = buffer_rx[0];

        int lungh_citta = bytes_ricevuti - 1;
        if (lungh_citta > 63) lungh_citta = 63;
        if (lungh_citta < 0) lungh_citta = 0;
        memcpy(req.city, buffer_rx + 1, lungh_citta);
        req.city[lungh_citta] = '\0';

        char* ip_cliente_str = inet_ntoa(client_addr.sin_addr);
        struct hostent* he_cliente = gethostbyaddr((const char*)&client_addr.sin_addr, sizeof(client_addr.sin_addr), AF_INET);
        char* nome_host_cliente;

        if (he_cliente != NULL) {
            nome_host_cliente = he_cliente->h_name;
        } else {
            nome_host_cliente = ip_cliente_str;
        }

        printf("Richiesta ricevuta da %s (ip %s): type='%c', city='%s'\n", nome_host_cliente, ip_cliente_str, req.type, req.city);

		weather_response_t res;
        res.status = STATUS_OK;
        res.type = req.type;
        res.value = 0.0f;

        if(req.type != 't' && req.type != 'h' && req.type != 'w' && req.type != 'p'){
            res.status = STATUS_INVALID_REQUEST;
        } else {
            const char* lista[] = { "bari", "roma", "milano", "napoli", "torino", "palermo", "genova", "bologna", "firenze", "venezia"};
            int trovato = 0;
            char nome_citta_lower[64];
            strncpy(nome_citta_lower, req.city, 63);
            nome_citta_lower[63] = '\0';

            for(int k=0; nome_citta_lower[k]; k++) nome_citta_lower[k] = tolower(nome_citta_lower[k]);

            for(int i = 0; i < 10; i++){
                if(strcmp(nome_citta_lower, lista[i]) == 0){
                    trovato = 1;
                    break;
                }
            }

            if(!trovato){
                res.status = STATUS_CITY_NOT_FOUND;
            } else {
                switch(req.type) {
                    case 't': res.value = get_temperature(); break;
                    case 'h': res.value = get_humidity(); break;
                    case 'w': res.value = get_wind(); break;
                    case 'p': res.value = get_pressure(); break;
                }
            }
        }

        char buffer_tx[BUFFER_SIZE];
        int cursore = 0;

        uint32_t stato_rete = htonl(res.status);
        memcpy(buffer_tx + cursore, &stato_rete, sizeof(uint32_t));
        cursore += sizeof(uint32_t);

        memcpy(buffer_tx + cursore, &res.type, sizeof(char));
        cursore += sizeof(char);

        uint32_t valore_temp;
        memcpy(&valore_temp, &res.value, sizeof(float));
        valore_temp = htonl(valore_temp);
        memcpy(buffer_tx + cursore, &valore_temp, sizeof(uint32_t));
        cursore += sizeof(uint32_t);

        sendto(my_socket, buffer_tx, cursore, 0, (struct sockaddr*)&client_addr, lungh_client);
	}

	closesocket(my_socket);
	clearwinsock();
	return 0;
}
