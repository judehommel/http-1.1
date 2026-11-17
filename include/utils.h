#ifndef UTILS_H_ 
#define UTILS_H_

#include <stddef.h>
#include <sys/poll.h>

#define MAX_DATA_SIZE 1028

typedef struct {
    int size;
    int count;
    struct pollfd *pfds;
    short int *past_revents;
    size_t *reqs_size;
    char **reqs;
} conns_t;

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
void add_to_conns(conns_t *conns, int newfd);
void del_from_conns(conns_t *conns, int *i);
void append_req(char **reqs, int i, size_t *reqs_size, char *buf);

#endif
