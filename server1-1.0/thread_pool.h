#ifndef _DAREN_THREAD_POOL_H
#define _DAREN_THREAD_POOL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include  <string.h>
#include <errno.h>
#include <event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "workers.h"
#define DAREN_WORKER_INSERT 1
#define DAREN_WORKER_DELETE 2
#define DAREN_WORKER_UPDATE 3 
#define DAREN_WORKER_QUERY  4 
#define DAREN_WORKER_DUMP  5 

//一个连接句柄
typedef struct fd_node
{
	int fd;
	struct fd_node* pre;
	struct fd_node* next;
}FD_NODE;

//连接队列
typedef struct fd_queue
{
	FD_NODE* head;
	FD_NODE* tail;
}FD_QUEUE;

//队列的弹入与弹出
 void fd_push(FD_NODE**, FD_NODE**, int);
int fd_pop(FD_NODE**, FD_NODE**);

//线程基本信息
typedef struct {

	u_int thread_id;        //线程ID
	struct event_base *base;   //事件根基
	struct event notify_event;
	int notify_receive_fd;   
	int notify_send_fd;     
        pthread_mutex_t queue_lock; //连接队列锁 
	FD_QUEUE conn_queue;    //连接队列
        int flag;     //当前线程是否占用着队列
        int queue_num;
        FILE* f_binlog;
        struct event flush_ev;

}THREAD_INFO;

//设置非阻塞模式
 int setnonblock(int);

//线程池初始化
 void thread_pool_init (int max_thread_num);


/*向某个线程的连接队列中加入连接句柄*/
 void queue_add_item(int fd);


//线程函数
 void * thread_routine (void *arg);

#endif
