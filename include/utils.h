#ifndef UTILS_H_ 
#define UTILS_H_

#include <sys/poll.h>

typedef struct {
    char* method;
    char* path;
    char* protocolVersion;
} start_line_t;

typedef struct {
    char* key;
    char* value;
} hash_map_item;

typedef struct {
    hash_map_item* items;
    int capacity;
} hash_map;

typedef struct {
    start_line_t *start_line;
    hash_map *headers; 
} parsed_req_t;

void sigchld_handler(int s);
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

#endif
