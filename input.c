// Artur Jankowski, 317928
#include <arpa/inet.h>   // pton, ntop et.c
#include <errno.h>       // error handling
#include <netinet/ip.h>  // sockaddr_in
#include <stdbool.h>     // bool's
#include <stdio.h>
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

// takes IPv4 address as string and converts it to network format in recipient
static inline void convert_pton(char *ip_addr, struct sockaddr_in *recipient) {
  recipient->sin_family = AF_INET;

  int result = inet_pton(AF_INET, ip_addr, &(recipient->sin_addr));
  if (result == 0) {
    fprintf(stderr, "%s is not a valid IPv4 address!\n", ip_addr);
    exit(EXIT_FAILURE);
  } else if (result < 0) {
    fprintf(stderr, "error during ip to binary conversion!: %s\n",
            strerror(errno));
    exit(EXIT_FAILURE);
  }
}

// converts string to number with error checking
static long convert_to_number(char *number_str) {
  char *end;
  long number = strtol(number_str, &end, 0);
  if (*end != '\0') {
    fprintf(stderr, "%s is not a number!\n", number_str);
    exit(EXIT_FAILURE);
  }

  return number;
}

// gets port from string and checks if it is valid
static uint32_t get_port(char *number_str) {
  long port = convert_to_number(number_str);

  if (port < 0 || port > 65535) {
    fprintf(stderr, "%s is not a valid port number ([0, 65535])!\n",
            number_str);
    exit(EXIT_FAILURE);
  }

  return (uint32_t)port;
}

// gets size from string and checks if it is in range
static uint32_t get_size(char *number_str) {
  long size = convert_to_number(number_str);

  if (size <= 0 || size > 10001000) {
    fprintf(stderr, "%s is not a valid file size ([1, 10001000])!\n",
            number_str);
    exit(EXIT_FAILURE);
  }
  return (uint32_t)size;
}

// opens file for writing, binary
static FILE *get_file(char *filename) {
  FILE *f = fopen(filename, "wb");
  if (f == NULL) {
    fprintf(stderr, "error opening file: %s,  %s\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  return f;
}

// returns net socket
static int server_prepare(struct sockaddr_in *server_address, uint32_t port) {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "socket error: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  server_address->sin_family = AF_INET;
  server_address->sin_port = htons((short)port);

  // binds automatically by sendto

  return sockfd;
}

void parse_input(char **argv, struct sockaddr_in *recipient, FILE **file,
                 uint32_t *total_size, int *sockfd) {
  uint32_t port = get_port(argv[2]);
  *file = get_file(argv[3]);
  *total_size = get_size(argv[4]);

  memset(recipient, 0, sizeof(*recipient));
  convert_pton(argv[1], recipient);
  *sockfd = server_prepare(recipient, port);
  debug("ip: %X, port: %u, file: %s, size: %u", recipient->sin_addr.s_addr,
        port, argv[3], *total_size);
}