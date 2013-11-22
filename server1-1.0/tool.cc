#include "tool.h"

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
