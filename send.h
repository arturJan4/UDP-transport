// Artur Jankowski, 317928
#ifndef SEND_HEADER_FILE
#define SEND_HEADER_FILE

#include <arpa/inet.h>   // pton, ntop et.c
#include <netinet/ip.h>  // sockaddr_in

#include "segment.h"

// limits SWS for smaller files
void update_window_limit(uint32_t total_size);

// sends i-th segment
void send_ith_segment(int sockfd, struct sockaddr_in *recipient, int i,
                      uint32_t max_size);
// send requests from initial window and initialize the window
void initial_send(int sockfd, struct sockaddr_in *recipient, uint32_t size);
// send requests to appropriate segments from window
void retry_send(int sockfd, struct sockaddr_in *recipient, uint32_t size,
                uint32_t lar);

#endif