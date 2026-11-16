#include "../include/dataFlowHandler.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

parsed_req_t* parse_req(char* req) {
    parsed_req_t* parsed_req = malloc(sizeof(*parsed_req));

    int i=0;
    for (; req[i] != ' '; i++) {
        parsed_req->start_line->method[i] = req[i];
    }
    parsed_req->start_line->method[i] = '\0';

    return parsed_req;
}

void recv_req(int sd, conn_t *conns, int *conn_i, char **buf, size_t buf_size) {
    int nbytes = recv(conns->pfds[*conn_i].fd, *buf, buf_size, 0);

    int sender_fd = conns->pfds[*conn_i].fd;

    if (nbytes <= 0) {
        if (nbytes == 0) {
            printf("Connection Closed\n");
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(conns->pfds[*conn_i].fd);

        del_from_pfds(conns->pfds, *conn_i, &conns->count);

        // reexamine the slot we just deleted
        (*conn_i)--;

    } else {
        append_req(&conns->reqs, *conn_i, &conns->reqs_size, *buf);
        printf("%p\n", (void*)conns->reqs[*conn_i]);
    }
}

void send_resp(int sd, conn_t *conns, int *conn_i) {
    char send_msg[] = "Hello World";
    if (send(conns->pfds[*conn_i].fd, send_msg, strlen(send_msg), 0) == -1) {
        perror("send");

        close(conns->pfds[*conn_i].fd);
        del_from_pfds(conns->pfds, *conn_i, &conns->count);
        del_from_reqs(conns->reqs, *conn_i, &conns->count);
        (*conn_i)--;
    }
}


void handle_http_flow(int sd, conn_t *conns, int *conn_i) {
    char *request = calloc(MAX_DATA_SIZE, sizeof(char));
    recv_req(sd, conns, conn_i, &request, sizeof request);
    //printf("%s", request);
    //parse_req(*request);

    send_resp(sd, conns, conn_i);
}

