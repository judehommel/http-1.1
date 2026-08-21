#ifndef DATAFLOW_H_ 
#define DATAFLOW_H_

#include <sys/poll.h>

void handle_http_flow(int sd, int *fd_count, struct pollfd *pfds, int *pfd_i);

#endif
