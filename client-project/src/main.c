/*
 * client.c
 *
 * UDP Client - Meteo Service
 */

#define MAXREQLENGHT 128
#define MAXADDRLENGHT 256

#if defined WIN32
#include <winsock2.h>
#include <stdint.h>
#else
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
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

void stampa_citta_formattata(const char* citta){
    int i = 0;
	while(citta[i] != '\0'){
		if(i == 0) {
            printf("%c", toupper(citta[i]));
        } else {
		    printf("%c", tolower(citta[i]));
		}
        i++;
	}
}

int main(int argc, char *argv[]) {

	char* indirizzo_server = "localhost";
	long port = SERVER_PORT;
	char* richiesta_input = NULL;

	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-s") == 0 && i + 1 < argc){
			indirizzo_server = argv[i + 1];
            i++;
		}
		else if(strcmp(argv[i], "-p") == 0 && i + 1 < argc){
		    port = strtol(argv[i + 1], NULL, 10);
            i++;
		}
		else if(strcmp(argv[i], "-r") == 0 && i + 1 < argc){
			richiesta_input = argv[i  + 1];
            i++;
		}
	}

	if(richiesta_input == NULL) {
        errorhandler("Il parametro -r e' obbligatorio");
        return -1;
    }

    char tipo_richiesto = '\0';
    char nome_citta[64] = {0};

    if (strchr(richiesta_input, '\t') != NULL) {
        errorhandler("Errore: La richiesta contiene caratteri di tabulazione non ammessi.");
        return -1;
    }

    char* primo_spazio = strchr(richiesta_input, ' ');
    if (primo_spazio == NULL) {
        errorhandler("Errore: Formato richiesta invalido. Usare \"type city\".");
        return -1;
    }

    int len_tipo = primo_spazio - richiesta_input;
    if (len_tipo != 1) {
        errorhandler("Errore: Il tipo richiesta deve essere un singolo carattere.");
        return -1;
    }
    tipo_richiesto = richiesta_input[0];

    char* inizio_citta = primo_spazio + 1;
    if (strlen(inizio_citta) >= 64) {
         errorhandler("Errore: Nome citta' troppo lungo (max 63 caratteri).");
         return -1;
    }
    strcpy(nome_citta, inizio_citta);
    if (strlen(nome_citta) == 0) {
        errorhandler("Errore: Nome citta' mancante.");
        return -1;
    }

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Errore su WSAStartup()");
		return 0;
	}
#endif

	int my_socket;
	my_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(my_socket < 0) {
        errorhandler("Creazione socket fallita");
        clearwinsock();
        return -1;
    }

	struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

    struct hostent *he_destinazione;
    if ((he_destinazione = gethostbyname(indirizzo_server)) == NULL) {
        errorhandler("Risoluzione host fallita");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }
    memcpy(&server_addr.sin_addr, he_destinazione->h_addr_list[0], he_destinazione->h_length);

    char buffer_tx[BUFFER_SIZE];
    buffer_tx[0] = tipo_richiesto;
    strcpy(buffer_tx + 1, nome_citta);
    int dim_richiesta = 1 + strlen(nome_citta) + 1;

	if(sendto(my_socket, buffer_tx, dim_richiesta, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		errorhandler("sendto() fallito");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

    char buffer_rx[BUFFER_SIZE];
    struct sockaddr_in addr_mittente;
    int len_mittente = sizeof(addr_mittente);

	int dim_risposta = recvfrom(my_socket, buffer_rx, BUFFER_SIZE, 0, (struct sockaddr*)&addr_mittente, &len_mittente);
    if(dim_risposta < 0){
        errorhandler("recvfrom() fallito");
		closesocket(my_socket);
		clearwinsock();
		return -1;
    }

    if (dim_risposta < 9) {
        errorhandler("Risposta non valida (troppo corta).");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    int cursore = 0;

    uint32_t stato_rete;
    memcpy(&stato_rete, buffer_rx + cursore, sizeof(uint32_t));
    unsigned int stato = ntohl(stato_rete);
    cursore += sizeof(uint32_t);

    char tipo_risp;
    memcpy(&tipo_risp, buffer_rx + cursore, sizeof(char));
    cursore += sizeof(char);

    uint32_t val_rete;
    float valore;
    memcpy(&val_rete, buffer_rx + cursore, sizeof(uint32_t));
    val_rete = ntohl(val_rete);
    memcpy(&valore, &val_rete, sizeof(float));
    cursore += sizeof(uint32_t);

    char* ip_mittente_str = inet_ntoa(addr_mittente.sin_addr);
    struct hostent* he_mittente = gethostbyaddr((const char*)&addr_mittente.sin_addr, sizeof(addr_mittente.sin_addr), AF_INET);
    char* nome_host_mittente;

    if (he_mittente != NULL) {
        nome_host_mittente = he_mittente->h_name;
    } else {
        nome_host_mittente = ip_mittente_str;
    }

	printf("Ricevuto risultato dal server %s (ip %s). ", nome_host_mittente, ip_mittente_str);

	if(stato == STATUS_OK){
		stampa_citta_formattata(nome_citta);
		switch(tipo_risp){
		case 't':
			printf(": Temperatura = %.1f°C\n", valore);
			break;
		case 'h':
			printf(": Umidità = %.1f%c\n", valore, 37);
			break;
		case 'w':
			printf(": Vento = %.1f km/h\n", valore);
			break;
		case 'p':
			printf(": Pressione = %.1f hPa\n", valore);
			break;
        default:
            printf(": Tipo dato sconosciuto\n");
            break;
		}
    } else if(stato == 1){
        printf("Città non disponibile\n");
    } else if(stato == 2){
        printf("Richiesta non valida\n");
    }

    closesocket(my_socket);
    clearwinsock();
    return 0;
}
