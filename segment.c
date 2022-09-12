// Artur Jankowski, 317928
#include "segment.h"

#include <errno.h>
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

struct segment segments[SWS];  // circular array

int get_segment_idx_from_frame(int frame_idx) { return frame_idx % SWS; }

int get_segment_idx(int byte_num) {
  return (byte_num / REQUEST_PACKET_SIZE) % SWS;
}

void update_segment_time(int i) {
  size_t idx = get_segment_idx_from_frame(i);
  gettimeofday(&segments[idx].last_send_try, NULL);
}

// returns how many seconds passed for i-th frame
static double elapsed_time(int i) {
  size_t idx = get_segment_idx_from_frame(i);
  struct timeval t1, t2;

  t1 = segments[idx].last_send_try;
  gettimeofday(&t2, NULL);

  double elapsed_time_calc = 0.0;
  elapsed_time_calc = (t2.tv_sec - t1.tv_sec) * 1000.0;     // sec to ms
  elapsed_time_calc += (t2.tv_usec - t1.tv_usec) / 1000.0;  // us to ms
  return elapsed_time_calc;
}

bool should_retry_segment(int i, uint32_t segment_cnt) {
  if (is_ack(i)) return false;

  if ((uint32_t)segments[i].frame_num >= segment_cnt) return false;

  double elapsed = elapsed_time(i);

  return (elapsed >= TIMEOUT_RETRY);
}

bool is_ack(int frame_idx) {
  size_t seg_idx = get_segment_idx_from_frame(frame_idx);

  return segments[seg_idx].is_ack;
}

static uint32_t get_segment_len(int i, uint32_t total_bytes) {
  // current segment frame number
  uint32_t frame = segments[i].frame_num;
  // total bytes before downloading this frame
  uint32_t len = frame * REQUEST_PACKET_SIZE;
  uint32_t len_next = (frame + 1) * REQUEST_PACKET_SIZE;
  // edge case where between total size is between
  // (segments[i].frame_num * REQUEST_PACKET_SIZE, (segments[i].frame_num+1) *
  // REQUEST_PACKET_SIZE)

  if (len_next > total_bytes) {
    return total_bytes - len;
  } else {
    return REQUEST_PACKET_SIZE;
  }
}

void try_move_window(uint32_t *lar, uint32_t segment_cnt, FILE **file,
                     uint32_t *written_bytes, uint32_t total_bytes) {
  size_t idx = get_segment_idx_from_frame((*lar + 1));

  while (idx < segment_cnt && segments[idx].is_ack) {
    // write prefix to file
    debug("wrtiting, idx: %lu, lar: %ld", idx, *lar);
    uint32_t len = get_segment_len(idx, total_bytes);

    size_t len_fwrite = fwrite(segments[idx].packet, sizeof(char), len, *file);
    if (len_fwrite != len) {
      fprintf(stderr, "fwrite error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    *written_bytes += len;

    // array loops (it represents new frame)
    segments[idx].is_ack = false;
    segments[idx].frame_num += SWS;

    *lar += 1;
    idx = get_segment_idx_from_frame((*lar + 1));
  }
}