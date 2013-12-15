#ifndef _RB_SIGNAL_H
#define _RB_SIGNAL_H
#include <sys/signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

void set_pipe(int p);

//阻塞所有的信号
void block_all_signal();


//主线程信号处理
void main_thread_sig_hand();

//信号是否在信号掩码中
int is_member(int sig);

#endif
