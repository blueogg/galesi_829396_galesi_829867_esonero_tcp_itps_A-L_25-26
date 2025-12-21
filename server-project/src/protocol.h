/*
 * protocol.h
 *
 * Header file comune per Client e Server UDP
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#if defined WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define SERVER_PORT 56700
#define BUFFER_SIZE 512

#define STATUS_OK 0
#define STATUS_CITY_NOT_FOUND 1
#define STATUS_INVALID_REQUEST 2

typedef struct {
    char type;
    char city[64];
} weather_request_t;

typedef struct {
    unsigned int status;
    char type;
    float value;
} weather_response_t;

#endif /* PROTOCOL_H_ */
