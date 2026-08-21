#ifndef UTILS_H_ 
#define UTILS_H_

#include <sys/poll.h>

void sigchld_handler(int s);
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

#endif
