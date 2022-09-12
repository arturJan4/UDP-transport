// Artur Jankowski, 317928
#ifndef SEGMENT_HEADER_FILE
#define SEGMENT_HEADER_FILE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

// maximum sender window size (server holds at most 1000 datagrams)
#define SWS 1000
#define TIMEOUT_RETRY 0.5         // timeout to retry sending a request
#define REQUEST_PACKET_SIZE 1000  // maximum size of requested packet

// segment in sender window
struct segment {
  uint32_t frame_num;                // frame number (0-indexed)
  bool is_ack;                       // received and accepted
  struct timeval last_send_try;      // when was last request sent
  char packet[REQUEST_PACKET_SIZE];  // binary data
};

// circular array
extern struct segment segments[SWS];

// given frame number returns index in circular window array
int get_segment_idx_from_frame(int frame_idx);
// given starting byte returns index in circular window array
int get_segment_idx(int byte_num);
// check if i-th frame is accepted
bool is_ack(int frame_idx);
// update time of i-th frarme
void update_segment_time(int i);
// check if we should send segment again
bool should_retry_segment(int i, uint32_t segment_cnt);

// try to move the window
void try_move_window(uint32_t *lar, uint32_t segment_cnt, FILE **file,
                     uint32_t *written_bytes, uint32_t total_bytes);

#endif