// Artur Jankowski, 317928
#include "send.h"

#include <errno.h>  // error handling
#include <math.h>
#include <netinet/ip.h>  // sockaddr_in
#include <stdbool.h>     // bool's
#include <stdio.h>       // printf, fpritnf
#include <stdlib.h>
#include <string.h>

/* Uncomment this line for debugging output */
//#define DEBUG
#ifdef DEBUG
#define debug(fmt, ...) printf("%s: " fmt "\n", __func__, __VA_ARGS__)
#define msg(...) printf(__VA_ARGS__)
#else
#define debug(fmt, ...)
#define msg(...)
#endif

// how many time to send same request
int repeat_num = 1;  // 1 - default
int limit = SWS;

void update_window_limit(uint32_t total_size) {
  if (REQUEST_PACKET_SIZE * SWS > total_size) {
    limit = ceil((double)total_size / REQUEST_PACKET_SIZE);
  }
}

// stores "GET [start] [length]\n" in buffer
static uint32_t prepare_request(uint32_t start, uint32_t length, char *buffer) {
  int32_t wr_len;
  wr_len = sprintf(buffer, "GET %u %u\n", start, length);
  if (wr_len < 0) {
    fprintf(stderr, "sprintf error for start: %u, length: %u, %s", start,
            length, strerror(errno));
    exit(EXIT_FAILURE);
  }

  return wr_len;
}

// sends request for "length" bytes starting from byte "start" to recipient
static void send_request(int sockfd, struct sockaddr_in *recipient,
                         uint32_t start, uint32_t length) {
  char message[24];  // arbitrary to fit: "GET 10000000 1000 \n"
  prepare_request(start, length, message);
  // debug("message: %s", message);

  size_t bytes_to_send = strlen(message);

  size_t bytes_sent = sendto(sockfd, message, bytes_to_send, 0,
                             (struct sockaddr *)recipient, sizeof(*recipient));

  // debug("bytes (%lu) sent: %ld to %X, port: %X", bytes_to_send, bytes_sent,
  //      recipient->sin_addr.s_addr, recipient->sin_port);

  if (bytes_sent != bytes_to_send) {
    fprintf(stderr, "Bytes sent don't match size of message. %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void send_ith_segment(int sockfd, struct sockaddr_in *recipient, int i,
                      uint32_t max_size) {
  uint32_t start = i * REQUEST_PACKET_SIZE;

  if (start > max_size) {
    fprintf(stderr, "start too big %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  uint32_t len = REQUEST_PACKET_SIZE;

  // last packet size may be smaller than previous
  if (start + len > max_size) {
    len = max_size - start;
  }

  update_segment_time(i);

  debug("sending request for %d-th segment", i + 1);
  send_request(sockfd, recipient, start, len);
}

void initial_send(int sockfd, struct sockaddr_in *recipient, uint32_t size) {
  // initalize segments
  for (int i = 0; i < limit; i++) {
    segments[i].is_ack = false;
    segments[i].frame_num = i;
  }

  if (limit < SWS) {
    repeat_num = (int)ceil((double)SWS / limit);
  }

  for (int repeat = 0; repeat < repeat_num; ++repeat) {
    // send each segment
    for (int i = 0; i < limit; i++) {
      send_ith_segment(sockfd, recipient, i, size);
    }
  }
}

void retry_send(int sockfd, struct sockaddr_in *recipient, uint32_t size,
                uint32_t lar) {
  // send each segment
  for (int i = 0; i < limit; i++) {
    int j = (i + (lar + 1));
    uint32_t segment_cnt = ceil((double)size / (double)REQUEST_PACKET_SIZE);
    if (should_retry_segment(j % limit, segment_cnt)) {
      if (i < 5)
        send_ith_segment(sockfd, recipient, j, size);
      
      if (i < 25)
        send_ith_segment(sockfd, recipient, j, size);

      send_ith_segment(sockfd, recipient, j, size);
    }
  }
}
