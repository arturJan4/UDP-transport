// Artur Jankowski, 317928
#include "receive.h"

#include <errno.h>  // error handling
#include <math.h>
#include <stdbool.h>  // bool's
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>  // timeval

#include "send.h"

uint32_t lar = -1;
uint32_t received_segments = 0;

/* Uncomment this line for debugging output */
//#define DEBUG
#ifdef DEBUG
#define debug(fmt, ...) printf("%s: " fmt "\n", __func__, __VA_ARGS__)
#define msg(...) printf(__VA_ARGS__)
#else
#define debug(fmt, ...)
#define msg(...)
#endif

// reads packet in form of "DATA [start] [length]\n..." in buffer and returns
// true on success, checks if it is a not yet received packet within the window
static bool read_received_packet(char *buffer) {
  uint32_t start, size;
  int ret = sscanf(buffer, "DATA %u %u\n", &start, &size);
  if (ret != 2) {
    debug("received message error: %s\n", strerror(errno));
    return false;
  }

  // different size
  uint32_t expected =
      segments[get_segment_idx(start)].frame_num * REQUEST_PACKET_SIZE;
  if (expected != start) {
    debug("expecting different starting size, got: %d, expected: %d", start,
          expected);
    return false;
  }

  // old packet
  if ((lar + 1) * REQUEST_PACKET_SIZE > start) {
    debug("old packet, lar: %d, start: %d", lar, start);
    return false;
  }

  // too far packet
  if (start > ((lar + 1) + SWS) * REQUEST_PACKET_SIZE) {
    debug("received data that is too far: %d\n", start);
    return false;
  }

  // already received
  size_t idx = get_segment_idx(start);
  if (segments[idx].is_ack) return false;

  // move pointer past '\n'
  size_t count_data = 0;
  while (buffer[count_data++] != '\n') {
  }

  memcpy(segments[idx].packet, &buffer[count_data], size);

  segments[idx].is_ack = true;
  received_segments++;
  debug("new received segment: idx: %lu, start: %u, len: %u", idx, start, size);
  return true;
}

// receives packets and validates [ip, port] pair
static bool receive_packet(int sockfd, struct sockaddr_in *recipient) {
  struct sockaddr_in sender;
  socklen_t sender_len = sizeof(sender);
  char buffer[IP_MAXPACKET];

  ssize_t packet_len = recvfrom(sockfd, buffer, IP_MAXPACKET, 0,
                                (struct sockaddr *)&sender, &sender_len);
  if (packet_len < 0) {
    fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (recipient->sin_addr.s_addr != sender.sin_addr.s_addr) {
    fprintf(stderr, "receive IP mismatch: %s\n", strerror(errno));
    return false;
  }

  if (recipient->sin_port != sender.sin_port) {
    fprintf(stderr, "receive port mismatch: %s\n", strerror(errno));
    return false;
  }

  // read packet from correct server
  return read_received_packet(buffer);
}

// receives packets using select() for wait until TIMEOUT
static void receive_packets(int sockfd, struct sockaddr_in *recipient) {
  struct timeval tv_to;
  tv_to.tv_sec = 0;
  tv_to.tv_usec = TIMEOUT;

  fd_set descriptors;
  FD_ZERO(&descriptors);
  FD_SET(sockfd, &descriptors);

  int ready;
  while ((ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv_to)) > 0) {
    if (ready > 0) receive_packet(sockfd, recipient);

    FD_ZERO(&descriptors);
    FD_SET(sockfd, &descriptors);
  }
}

// prints partial progress
static inline void print_progress(uint32_t written_bytes, uint32_t total,
                                  uint32_t segment_cnt) {
  printf("Progress: %.2f%% written (received %d segments out of %d needed)\n",
         ((double)written_bytes / total) * 100, received_segments, segment_cnt);
}

// download packets and send request while moving the window
void transport_loop(int sockfd, struct sockaddr_in *recipient, FILE **file,
                    uint32_t size) {
  // how many bytes were already written to file
  uint32_t written_bytes = 0;
  // how many segments were already received
  received_segments = 0;
  lar = -1;  // last ACK received (0-indexed)

  // how many segments are needed in total
  uint32_t segment_cnt = ceil((double)size / (double)REQUEST_PACKET_SIZE);

  debug("segment_cnt: %u", segment_cnt);
  // if size of file is smaller than cumulative window size
  update_window_limit(size);

  // send initial packets from segment window
  initial_send(sockfd, recipient, size);

  while (lar + 1 < segment_cnt) {
    // print how many percent written and how many segments downloaded
    print_progress(written_bytes, size, segment_cnt);
    // received packets in the window
    receive_packets(sockfd, recipient);
    // try to move window forward and save to file
    try_move_window(&lar, segment_cnt, file, &written_bytes, size);
    // send packets from window
    retry_send(sockfd, recipient, size, lar);
  }

  print_progress(written_bytes, size, segment_cnt);
}