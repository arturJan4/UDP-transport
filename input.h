// Artur Jankowski, 317928
#ifndef INPUT_HEADER_FILE
#define INPUT_HEADER_FILE

// parsing input from user
void parse_input(char **argv, struct sockaddr_in *recipient, FILE **file,
                 uint32_t *total_size, int *sockfd);

#endif