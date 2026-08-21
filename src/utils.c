#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include "../include/utils.h"

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
    start_line_t start_line;
    hash_map headers; 
} parsed_req_t;

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_key(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

hash_map_item* hm_create() {
    hash_map_item *map = calloc(16, sizeof(hash_map_item));

    return map;
}

void hm_set(hash_map_item** map, char* key, char* value) {
    for (int i=hash_key(key); i<0; i++) {
        if (map[i]->key == NULL) {
            strcpy(map[i]->key, key);
            strcpy(map[i]->value, value);
            return;
        }

        if (strcmp(key, map[i]->key)) {
            strcpy(map[i]->value, value);
            return;
        }
    }
}

void sigchld_handler(int s) {
    (void)s;

    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size) {
    if (*fd_count == *fd_size) {
        *fd_size *= 2;
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}
