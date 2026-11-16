#ifndef UTILS_H_ 
#define UTILS_H_

#include <stddef.h>
#include <sys/poll.h>

#define MAX_DATA_SIZE 1028

typedef struct {
    int fd_size;
    int count;
    struct pollfd *pfds;
    size_t *reqs_size;
    char **reqs;
} conn_t;

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
void add_to_reqs(char ***reqs, int *req_count, size_t **req_size);
void del_from_reqs(char **reqs, int i, int *req_count);
void append_req(char ***req, int req_count, size_t **req_size, char *buf);

#endif
