#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include "../include/dataFlowHandler.h"
#include "../include/utils.h"

#define PORT "8080"
#define BACKLOG 10

int init_socket() {
    struct addrinfo hints, *servinfo, *p;
    int sd, rv;
    int yes=1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }

        if (bind(sd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return -1;
    }

    if (listen(sd, BACKLOG) == -1) {
        perror("listening error");
        return -1;
    }

    return sd;
}

void handle_new_connection(int sd, conn_t *conns) {
    struct sockaddr_storage client_addr;
    socklen_t addrlen;
    int client_fd;

    addrlen = sizeof client_addr;
    client_fd = accept(sd, (struct sockaddr *)&client_addr, &addrlen);

    if (client_fd == -1) {
        perror("accept");
    } else {
        add_to_pfds(&conns->pfds, client_fd, &conns->count, &conns->fd_size);
        add_to_reqs(&conns->reqs, &conns->count, &conns->reqs_size);
        conns->count++;

        printf("Client Connected\n");
    }
}

void process_connections(int sd, conn_t *conns) {
    for(int i = 0; i < conns->count; i++) {
        if ((conns->pfds)[i].revents & (POLLIN | POLLOUT | POLLHUP)) {
            if ((conns->pfds)[i].fd == sd) {
                handle_new_connection(sd, conns);
            } else {
                handle_http_flow(sd, conns, &i);
            }
        }
    }
}

int main() {
    int sd;

    conn_t *conns = malloc(sizeof *conns);
    conns->fd_size = 5;
    conns->count = 0;
    // Allocate memory for array of pfds
    conns->pfds = malloc(sizeof *(conns->pfds) * conns->fd_size);
    // Allocate memory for array of requests (strings)
    conns->reqs = malloc(sizeof *(conns->pfds) * conns->fd_size);
    
    if ((sd = init_socket()) == -1) {
        perror("initializing socket");
        exit(1);
    }

    struct sigaction sa;

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    conns->pfds[0].fd = sd;
    conns->pfds[0].events = POLLIN;

    conns->count = 1;

    printf("server: waiting for connections...\n");

    while (1) {
        int poll_count = poll(conns->pfds, conns->count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        process_connections(sd, conns);
    }

    free(conns->pfds);
    for (int i=0; i<conns->count; i++) {
        free(conns->reqs[i]);
    }
    free(conns->reqs);
    free(conns);
    return 0;
}
