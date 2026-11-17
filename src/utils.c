#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include "../include/utils.h"

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

void add_to_conns(conns_t *conns, int newfd) {
    if (conns->count == conns->size) {
        conns->size *= 2;
        conns->pfds = realloc(conns->pfds, sizeof(*(conns->pfds)) * (conns->size));
        conns->reqs = realloc(conns->reqs, sizeof(*(conns->reqs)) * (conns->size));
        conns->reqs_size = realloc(conns->reqs_size, sizeof(*(conns->reqs_size)) * (conns->size));

        if (conns->pfds == NULL) {
            perror("could not realloc pfds");
        }
        if (conns->reqs == NULL) {
            perror("could not realloc reqs");
        }
        if (conns->reqs_size == NULL) {
            perror("could not realloc reqs_size");
        }
    }

    conns->pfds[conns->count].fd = newfd;
    conns->pfds[conns->count].events = POLLIN;
    conns->pfds[conns->count].revents = 0;

    conns->reqs_size[conns->count] = 1024;
    conns->reqs[conns->count] = malloc(sizeof(**(conns->reqs)) * conns->reqs_size[conns->count]);
    if (conns->reqs[conns->count] == NULL) {
        perror("could not malloc reqs[conns->count]");
    }
    *conns->reqs[conns->count] = '\0';

    assert(conns->pfds != NULL);

    conns->count++;
}

void del_from_conns(conns_t *conns, int i)
{
    conns->pfds[i] = conns->pfds[conns->count-1];

    conns->count--;
}

/**
 * @param char **reqs An array of string requests
 * @param int i The index for the array of requests
 * @param size_t **reqs_size An array of the lengths of requests
 * @param char *buf A string that contains a part of the full request 
 */
void append_req(char **reqs, int i, size_t *reqs_size, char *buf) {
    if (strlen(reqs[i]) + strlen(buf) > reqs_size[i]) { 
        reqs_size[i] *= 2;
        reqs[i] = realloc(reqs[i], sizeof(reqs[i]) * (reqs_size[i]));
        if (reqs[i] == NULL) {
            perror("could not realloc reqs[i]");
        }
    }

    strncat(reqs[i], buf, reqs_size[i]);
}
