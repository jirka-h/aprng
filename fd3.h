/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

#ifndef _FD3_H
#define _FD3_H

uint64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);
void run_fd3(unif01_Gen *g, const char* path, unsigned long long TEST);

#endif

