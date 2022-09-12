// Artur Jankowski, 317928
#include <arpa/inet.h>  // pton, ntop et.c
#include <stdio.h>      // printf, fpritnf
#include <stdlib.h>
#include <unistd.h>  // close

#include "input.h"
#include "receive.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    printf("Usage: %s [ip addr] [port] [filename] [size]\n", argv[0]);
    return EXIT_FAILURE;
  }

  struct sockaddr_in recipient;  // server address with port
  FILE *file;
  uint32_t size;  // size-to-download in bytes
  int sockfd;     // sending/receiving socket

  // parse input and prepare socket
  parse_input(argv, &recipient, &file, &size, &sockfd);
  // downloading loop
  transport_loop(sockfd, &recipient, &file, size);

  fclose(file);
  close(sockfd);
}
