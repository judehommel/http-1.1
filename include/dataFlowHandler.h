#ifndef DATAFLOW_H_ 
#define DATAFLOW_H_

#include "utils.h"
#include <stddef.h>
#include <sys/poll.h>

void handle_http_flow(int sd, conns_t *conns, int *conn_i);

#endif
