#include "thread_pool.h"

static THREAD_INFO* threads= NULL;
static int thread_num = 0;
static int begin_thread_id=0;
static struct event ev_time;
static struct timeval tv;
static int today_dump=0;

int setnonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}
void fd_push(FD_NODE** head, FD_NODE** tail, int fd)
{
	FD_NODE* new_node = (FD_NODE*)malloc(sizeof(FD_NODE));
	new_node -> fd=fd;
	new_node -> pre=NULL;
	new_node -> next=NULL;
	//如果队列为空
	if(!*head || !*tail)
		*head=*tail=new_node;
	(*head)->pre=new_node;
	new_node->next=*head;
	*head=new_node;
}

static void write_binlog(u_int opcode, u_long oid, u_int score, u_int time, FILE* f)
{
	char buf[21];
	sprintf(buf, "%u%lu%u%u\n", opcode, oid, score, time);
	fwrite(buf, 1, 21, f);
}

int fd_pop(FD_NODE** head, FD_NODE** tail)
{
	FD_NODE* target=NULL;
	//如果队列为空
	if(!*head || !*tail)
		return -1;
	//如果队列中只有一个节点
	if(*head==*tail)
	{
		target=*tail;
		*head=*tail=NULL;
	}
	else {
		target=*tail;
		*tail = target -> pre;
	}
	int fd=target->fd;
	free(target);
	return fd;
}

void time_cb(int fd,short event,void *argc)
{
        char* config_dump_hour = get_config_var("dump_hour"); 
        int dump_hour = config_dump_hour ? atoi(config_dump_hour) : 4;

	time_t timer;
	struct tm *timeinfo;
	timer = time(NULL);
	timeinfo= localtime(&timer);
	if(timeinfo->tm_hour == dump_hour)
	{
		if(!today_dump)
		{
                        fprintf(stderr, "dumping...trigger by timer\n");
			save_nodes_to_file(NULL);
			today_dump=1;
		}

	} else
		today_dump = 0;

	event_add(&ev_time,&tv);
}

void flush_cb(int fd,short event,void *argc)
{
        THREAD_INFO* info=(THREAD_INFO*)argc;
        struct timeval tv_flush;
	tv_flush.tv_sec=10; //间隔
	tv_flush.tv_usec=0;
        fflush(info->f_binlog);
	event_add(&info->flush_ev,&tv_flush);
}

void thread_sock_write_callback(int fd, short ev, void *arg)
{
	char* result = (char*)arg;
	send(fd, result, strlen(result), 0);
	close(fd);
	free(result);
}
void thread_sock_read_callback(int fd, short ev, void *arg)
{
	if (ev == EV_TIMEOUT) {
		close(fd);
		return;
	}
	int read_bytes;
	char buffer[28];
	do {
		read_bytes = recv(fd, buffer, sizeof(buffer), 0);
	} while (read_bytes == -1 && errno == EINTR);

	if (read_bytes == 0) {
		close(fd);
		return;
	} else if (read_bytes < 0) {
		fprintf(stderr, "Failed read request '%s'", strerror(errno));
		close(fd);
		return;
	}
	int opcode,oid,score,time;
	//密码验证
	int i=0;
	char* pass="220106aa";
	for(;i<8;i++)
		if(buffer[i] != pass[i])
			break;
	if(i != 8)
	{
		close(fd);
		fprintf(stderr,"request with wrong password\n");
		return;
	}
	char* loc=buffer+8;
	opcode= ntohl(*(int*)loc);loc+=4;
	oid= ntohl(*(u_long*)loc);loc+=8;
	score= ntohl(*(int*)loc);loc+=4;
	struct timeval tv;
	gettimeofday(&tv, 0);
	time = tv.tv_sec; 

	ss_input input;
	input.oid=oid ;
	input.score= score;
	input.time= time;
	char* result;
	THREAD_INFO* info = (THREAD_INFO*)arg;
	switch(opcode)
	{
		case DAREN_WORKER_INSERT:
                        write_binlog(opcode,oid,score,time,info->f_binlog); 
			result=insert_node_s((void*)&input);
			break;
		case DAREN_WORKER_DELETE:
                        write_binlog(opcode,oid,score,time,info->f_binlog); 
			result=delete_node_s((void*)&input);
			break;
		case DAREN_WORKER_UPDATE:
                        write_binlog(opcode,oid,score,time,info->f_binlog); 
			result=update_node_s((void*)&input);
			break;
		case DAREN_WORKER_QUERY:
			result=query_node_s((void*)&input);
			break;
		case DAREN_WORKER_DUMP:
			save_nodes_to_file(NULL);
			return;
		default:
			close(fd);
			fprintf(stderr,"request with wrong opcode\n");
			return;
	}
	//添加socket写事件
	struct event *sock_write= (struct event*)malloc(sizeof(struct event));
    event_set(sock_write,
                        fd,
                        EV_WRITE,
                        thread_sock_write_callback,
                        (void*)result);

        event_base_set(info->base, sock_write);
        event_add(sock_write,
                        NULL);

	//struct event *sock_write=event_new(info->base, fd, EV_WRITE, thread_sock_write_callback, (void*)result);
	event_add(sock_write, 0); 
}
void thread_pipe_read_callback(int fd, short ev, void *arg)
{
	THREAD_INFO* info = (THREAD_INFO*)arg;
	char buf[1];
	read(fd, buf, 1);

	//读取一个连接
	pthread_mutex_lock(&info->queue_lock);
	info->flag=1;
	int client_fd = fd_pop(&info->conn_queue.head, &info->conn_queue.tail);
        info->queue_num--;
	pthread_mutex_unlock(&info->queue_lock);
	info -> flag=0;
	if(client_fd == -1)
		return;

	//添加socket读事件
	struct event *sock_read = (struct event*)malloc(sizeof(struct event));
    event_set(sock_read,
                        client_fd,
                        EV_READ,
                        thread_sock_read_callback,
                        arg);

        event_base_set(info->base, sock_read);
        event_add(sock_read,
                        NULL);

//struct event *sock_read=event_new(info->base, client_fd, EV_READ, thread_sock_read_callback, arg);
	struct timeval tv;
	tv.tv_sec=2;
	tv.tv_usec=0;
	event_add(sock_read, &tv); 
}

void queue_add_item(int fd)
{	//成功插入
	int add_success=0;
	int loop_num=0;
	while(!add_success)
	{
		if(++loop_num>1)
			begin_thread_id=0;;
		if(begin_thread_id > thread_num-1)
			begin_thread_id=0;
		for(int i=begin_thread_id;i<thread_num;i++)
		{
			//初步判断缓冲区是否被占用
			if(threads[i].flag>0)continue;
			//再次尝试加锁
			if(pthread_mutex_trylock(&threads[i].queue_lock) != 0)continue;
			fd_push(&threads[i].conn_queue.head, &threads[i].conn_queue.tail, fd);
                        threads[i].queue_num++;
			//加锁成功
			begin_thread_id = i+1;
			pthread_mutex_unlock(&threads[i].queue_lock);
			write(threads[i].notify_send_fd, " ", 1);
			add_success=1;
			break;
		}
	}
}
//初始化一个线程
static void setup_thread(THREAD_INFO* me) 
{
	int fds[2];
	pipe(fds);
	me->notify_receive_fd = fds[0];
	me->notify_send_fd = fds[1];
	me->base = event_init();
	event_set(&me->notify_event, me->notify_receive_fd, EV_READ | EV_PERSIST, thread_pipe_read_callback, (void*)me);
	event_base_set(me->base, &me->notify_event);
	event_add(&me->notify_event, 0);
	me->conn_queue.head=NULL;
	me->conn_queue.tail=NULL;
	pthread_mutex_init(&me->queue_lock, NULL);
	me->flag=0;
        me->queue_num=0;
        char* binlog_path = get_config_var("binlog_path");
        if(!binlog_path)binlog_path="/tmp";
        char buf[100];
        sprintf(buf,"%s/binlog_%d", binlog_path, me->thread_id);
        me->f_binlog=fopen(buf,"a+");
}

//线程池初始化
void thread_pool_init(int max_thread_num)
{
	threads = (THREAD_INFO*)malloc(sizeof(THREAD_INFO)*max_thread_num);
	thread_num = max_thread_num;

	for (int i = 0; i < max_thread_num; i++)
	{
                threads[i].thread_id=i;
		setup_thread(&threads[i]);
                pthread_t t_id;
		pthread_create (&t_id, NULL, thread_routine,
				(void*)(&threads[i]));
	}
}




void * thread_routine (void *arg)
{
	THREAD_INFO* info = (THREAD_INFO*)arg;
        //如果是第一个线程，为它添加定时dump nodes事件
	if(info->thread_id==0)
	{
		tv.tv_sec=10; //间隔
		tv.tv_usec=0;
		evtimer_set(&ev_time,time_cb,NULL);
		event_base_set(info->base, &ev_time);
		event_add(&ev_time,&tv);
	}
        //为所有的线程添加flush事件
        struct timeval tv_flush;
	tv_flush.tv_sec=10; //间隔
	tv_flush.tv_usec=0;
	evtimer_set(&info->flush_ev,flush_cb,arg);
	event_base_set(info->base, &info->flush_ev);
	event_add(&info->flush_ev,&tv_flush);

	event_base_loop(info->base, 0);
}

