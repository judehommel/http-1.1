#include "../include/dataFlowHandler.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

struct parsed_req_t parse_req(char* req) {
    struct parsed_req_t* parsed_req = calloc(sizeof(struct parsed_req_t));

    int i=0;
    for (; req[i] != "/n"; i++) {
        parsed_req.start_line[i] = req[i];
    }
    parsed_req.start_line[i] = "/0";
    printf("%s\n", parsed_req.start_line);

    return parsed_req;
}

void recv_req(int sd, int *fd_count, struct pollfd *pfds, int *pfd_i, char **buf, size_t buf_size) {
    int nbytes = recv(pfds[*pfd_i].fd, *buf, buf_size, 0);

    int sender_fd = pfds[*pfd_i].fd;

    if (nbytes <= 0) {
        if (nbytes == 0) {
            printf("Connection Closed\n");
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(pfds[*pfd_i].fd);

        del_from_pfds(pfds, *pfd_i, fd_count);

        // reexamine the slot we just deleted
        (*pfd_i)--;

    } else {
        parse_req(buf);
    }
}

void send_resp(int sd, int *fd_count, struct pollfd *pfds, int *pfd_i) {
    char send_msg[] = "Hello World";
    if (send(pfds[*pfd_i].fd, send_msg, strlen(send_msg), 0) == -1) {
        perror("send");

        close(pfds[*pfd_i].fd);
        del_from_pfds(pfds, *pfd_i, fd_count);
        (*pfd_i)--;
    }
}


void handle_http_flow(int sd, int *fd_count, struct pollfd *pfds, int *pfd_i) {
    char *request = calloc(1028, sizeof(char));
    recv_req(sd, fd_count, pfds, pfd_i, &request, sizeof request);

    send_resp(sd, fd_count, pfds, pfd_i);
}

