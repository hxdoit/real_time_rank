#ifndef _DAREN_WORKERS_WORKERS_H
#define _DAREN_WORKERS_WORKERS_H
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "hash_map.h" 
#include "config_file.h" 
#include "node.h" 
// 输入输出统一化
// 输入
typedef struct _ss_input
{
  u_long oid;
  u_int score;
  u_int time;
}ss_input; 

//输出
typedef struct _ss_output
{
  u_int code;
  char* msg;
  u_int data;
}ss_output; 

/* 增、删、改、查*/
 void init_insert_node(void *arg);
 char* insert_node(void *arg);
 char* delete_node(void *arg);
 char* update_node(void *arg);
 char* query_node (void *arg);
 char* save_nodes_to_file(void *arg);

/* 线程安全：增、删、改、查*/
 char* insert_node_s(void *arg);
 char* delete_node_s(void *arg);
 char* update_node_s(void *arg);
 char* query_node_s (void *arg);

char* set_result(u_int,char*,u_int);
#endif
