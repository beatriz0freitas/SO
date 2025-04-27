#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

ssize_t bufferedRead(int fd, void *buf, size_t count);
ssize_t bufferedWrite (int fd, const void *buf, size_t count);
int mysystem(const char *command_line);
#endif 