#ifndef _SS_TOOL_H
#define _SS_TOOL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <printf.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#define LOCKFILE "/var/run/mydaemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
int already_running(const char*);
#endif
