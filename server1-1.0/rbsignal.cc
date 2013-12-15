#include "rbsignal.h" 

static int rb_pipe;
void set_pipe(int p)
{
	rb_pipe = p;
}

//阻塞所有的信号
void block_all_signal()
{
	sigset_t mask;
	sigfillset(&mask);
	int rc = pthread_sigmask(SIG_BLOCK, &mask, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "block all signal error: %s\n", strerror(rc));
		exit;
	}
}

//所有信号的处理函数
//就是向管道发一个信号值，以便在libevent循环中处理，目标：统一事件源
static void sig_handler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	int r = write(rb_pipe, (char*)&msg, 4);
	errno = save_errno;
}

//安装一个信号处理程序
static void add_signal(int sig)
{
   struct sigaction action;
   memset(&action, 0, sizeof(action));
   action.sa_handler = sig_handler;
   sigfillset(&action.sa_mask);
   sigaction(sig, &action, NULL);
}

//主线程的信号处理
void main_thread_sig_hand()
{
        sigset_t except;
        sigemptyset(&except);
        sigaddset(&except, SIGHUP);
        sigaddset(&except, SIGPIPE);
        sigaddset(&except, SIGTERM);
        sigaddset(&except, SIGINT);
	int rc= pthread_sigmask(SIG_UNBLOCK, &except, NULL);
	if (rc != 0)
	{
		fprintf(stderr, "main thread signal error: %s\n", strerror(rc));
		exit;
	}      
	add_signal(SIGHUP);
	add_signal(SIGPIPE);
	add_signal(SIGTERM);
	add_signal(SIGINT);
}
//信号是否在信号掩码中
int is_member(int sig)
{
	sigset_t old;
	pthread_sigmask(SIG_SETMASK, NULL, &old);
	return sigismember(&old, sig);
}
