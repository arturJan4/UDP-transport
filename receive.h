// Artur Jankowski, 317928
#ifndef RECEIVE_HEADER_FILE
#define RECEIVE_HEADER_FILE

#include <netinet/ip.h>  // sockaddr_in
#include <stdint.h>      // uint32_t
#include <stdio.h>       // FILE

#define TIMEOUT 10000  // timeout in microseconds (0.01s) for waiting for packet

extern uint32_t lar;

// main program's logic loop
void transport_loop(int sockfd, struct sockaddr_in *recipient, FILE **file,
                    uint32_t size);

#endif