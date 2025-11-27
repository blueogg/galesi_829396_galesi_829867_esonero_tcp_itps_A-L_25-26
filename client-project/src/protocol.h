/*
 * protocol.h
 *
 * Client header file
 * Definitions, constants and function prototypes for the client
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define SERVER_PORT 27015  // Server port (change if needed)
#define BUFFER_SIZE 512    // Buffer size for messages


typedef struct {
    char type;        // Weather data type: 't', 'h', 'w', 'p'
    char city[64];    // City name (null-terminated string)
} weather_request_t;

typedef struct {
    unsigned int status;  // Response status code
    char type;            // Echo of request type
    float value;          // Weather data value
} weather_response_t;


weather_request_t* requestCreate(char richiesta[]){

	weather_request_t* req = malloc(sizeof(weather_request_t));
	req->type = richiesta[0];
	char nome[64];
	int i = 2;
	int j = 0;
	while(richiesta[i] != '\0'){

		nome[j] = richiesta[i];
		i++;
		j++;
	}
	strcpy(req->city, nome);
	return req;
}

size_t getreqsize(){
	return sizeof(weather_request_t);
}

size_t getressize(){
	return sizeof(weather_request_t);
}


#endif /* PROTOCOL_H_ */
