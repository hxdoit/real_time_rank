/******* 服务器程序 (server.c) ************/    
//aa
#include  <stdio.h>   
#include  <stdlib.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include  <string.h>   
#include <netinet/in.h>   
#include <errno.h>   
#include <event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "thread_pool.h"   
#include "workers.h"   
#include "rbtree.h"   
#include "node.h"   
#include "config_file.h"   
#include "tool.h"   
#include "rbsignal.h"   
static int pipefd[2];
// 获取文件行数
unsigned int get_line_num(char *f)
{
	FILE*fr=fopen(f,"r");
	unsigned int num=0;
	while(!feof(fr))
		if(fgetc(fr)==10)
			++num;
	fclose(fr);
	return num;
}
//初始化时读文件插入
void init_insert(char *f)
{
	FILE *fp;
	char text[100];
	fp = fopen(f,"r");
	while(fgets(text,100,fp)!=NULL)
	{
		u_long oid; 
		int score, time;
		score=time=0;
		sscanf(text,"%lu %d %d", &oid, &score, &time);
		if(oid && time)
		{
			ss_input input;
			input.oid=oid;
			input.score=score;
			input.time=time;
			init_insert_node((void*)&input);
		}
	}
	fclose(fp);
}


void sig_callback(int fd,short ev,void *arg)
{
	char c[4];
	read(fd, c ,4);
	int sig = *(int*)c;
	switch(sig)
	{
		case SIGHUP:
			fprintf(stderr,"receive SIGHUP\n");  
			break;
		case SIGINT:
			fprintf(stderr,"receive SIGINT\n");  
			break;
		case SIGTERM:
			fprintf(stderr,"receive SIGTERM\n");  
			break;
		case SIGPIPE:
			fprintf(stderr,"receive SIGPIPE\n");  
			break;
	}
}
void accept_callback(int fd,short ev,void *arg)
{
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct client *client;

	client_fd = accept(fd,	(struct sockaddr *)&client_addr,&client_len);
	if (client_fd < 0)
	{
		fprintf(stderr,"accept failed\n");  
		return;
	}

	setnonblock(client_fd);
	queue_add_item(client_fd);
}


int main(int argc, char *argv[])    
{    
	//只允许运行一个实例
	if(already_running(LOCKFILE))
	{
		fprintf(stderr,"an instance is already running\n");  
		exit(1);
	}

	//读取配置文件
	parse_config_file("server.conf");
	char* c_thread_num=get_config_var("thread_num");
	char* c_listen_num=get_config_var("listen_num");
	int thread_num= c_thread_num ? atoi(c_thread_num) : 5;
	int listen_num= c_listen_num ? atoi(c_listen_num) : 1024;
	fprintf(stderr,"thread_num:%d\nlisten_num:%d\n",thread_num,listen_num);  

	int sockfd,new_fd;    
	struct sockaddr_in server_addr;    
	struct sockaddr_in client_addr;    
	int portnumber;    
	socklen_t sin_size;

	if(argc<3)    
	{    
		fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);    
		exit(1);    
	}    

	if((portnumber=atoi(argv[1]))<0)    
	{    
		fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);    
		exit(1);    
	}    

	/* 服务器端开始建立socket描述符 */    
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)    
	{    
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));    
		exit(1);    
	}    

	/* 服务器端填充 sockaddr结构 */    
	bzero(&server_addr,sizeof(struct sockaddr_in));    
	server_addr.sin_family=AF_INET;    
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);    
	server_addr.sin_port=htons(portnumber);    

	/* 捆绑sockfd描述符 */    
	if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)    
	{    
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));    
		exit(1);    
	}    

	/* 监听sockfd描述符 */    
	if(listen(sockfd,listen_num)==-1)    
	{    
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno));    
		exit(1);    
	}    

	/*信号处理*/
	block_all_signal();

	/*初始化线程池*/
	thread_pool_init(thread_num); 

	/*为主线程添加信号处理*/
	pipe(pipefd);
	set_pipe(pipefd[1]);
	main_thread_sig_hand();

	/*初始化存储空间,多申请20%的空间*/
	unsigned int line_num = get_line_num(argv[2]);
	buffer_init((int)(1.2*line_num)); 

	/*初始化插入*/
	init_insert(argv[2]);

	fprintf(stderr,"finish init insert,begin accept request...\n");    

	struct event accept_event;
	struct event sig_event;
	int reuse = 1;

	struct event_base* base=event_init();
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

	setnonblock(sockfd);
	setnonblock(pipefd[0]);

	event_set(&accept_event,sockfd, EV_READ|EV_PERSIST, accept_callback, NULL);
	event_set(&sig_event,pipefd[0], EV_READ|EV_PERSIST, sig_callback, NULL);

	event_base_set(base, &accept_event);
	event_base_set(base, &sig_event);

	event_add(&accept_event,NULL);
	event_add(&sig_event,NULL);

	event_base_loop(base, 0);

	close(sockfd);

	exit(0);    
}    
