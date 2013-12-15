#include "tool.h"
//服务器程序后台化
int daemonize()
{
        //创建父进程，创建子进程，使程序在后台运行
        pid_t pid=fork();
        if(pid<0)
                return -1;
        else if(pid >0)
                exit;

        //设置文件权限掩码
        umask(0);
        //创建新的会话，从而使程序脱离终端
        pid_t sid=setsid();
        if(sid<0)
                return -1;
        //切换工作目录
        if(chdir("/")<0)
                return -1;
        //关闭标准输入，输出，错误
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        //重定向标准输入，输出，错误
        open("/dev/null",O_RDONLY);
        open("/dev/null",O_RDWR);
        open("/dev/null",O_RDWR);
        return 0;
}
/* set advisory lock on file */
int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;  /* write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;  //lock the whole file

	return(fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		printf("can't open %s: %m\n", filename);
		exit(1);
	}

	/* 先获取文件锁 */
	if (lockfile(fd) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			printf("file: %s already locked", filename);
			close(fd);
			return 1;
		}
		printf("can't lock %s: %m\n", filename);
		exit(1);
	}
	/* 写入运行实例的pid */
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}
