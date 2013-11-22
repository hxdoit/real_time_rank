#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


//节点的大小
static u_int node_size;
//缓充区的开始
static node *buffer_start;
//缓充区的大小
static u_int buffer_size;
//当前使用的最后一个节点
static u_int cur_node;

//缓充区初始化
void buffer_init(u_int size)
{
	node_size = sizeof(node);
	buffer_start = (node*)malloc(node_size * (size + 1));
	buffer_size = size;
	cur_node = 1;
	if (buffer_start == NULL)
	{
		fprintf(stderr,"malloc error: %s\n",strerror(errno));
		exit(1);
	}
}

//创建一个节点
node* create_node()
{
	if (cur_node > buffer_size)
	{
		fprintf(stderr,"not enough buffer for a new node\n");
		exit(1);
	}  
        return &buffer_start[cur_node ++];
}


node* node1(u_int i)
{
        if (i==0)
            return NULL;
	return &buffer_start[i];
}

u_int id(node* p)
{
        if (p==NULL)
            return 0;
	return ((u_long)p - (u_long)buffer_start)/node_size; 
}
