#ifndef __BASIC__ 
#define __BASIC__

#include <sys/types.h> /* basic system data types */
#include <sys/socket.h> /* basic socket definitions */
#include <sys/time.h> /* timeval{} for select() */
#include <time.h> /* timespec{} for pselect() */
#include <netinet/in.h> /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h> /* inet(3) functions */
#include <errno.h>
#include <fcntl.h> /* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* for S_xxx file mode constants */
#include <sys/uio.h> /* for iovec{} and readv/writev */
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h> /* for Unix domain sockets */

#define MAXLINE 256
#define PORT 9890
#define BACKLOG 5 
#define MAX(a, b) ((a) > (b) ? (a) : (b))

ssize_t exso_read(int fd, char *ptr);
ssize_t exso_readln(int fd, void *vptr, size_t maxlen);
ssize_t exso_writen(int fd, const void *vptr, size_t n);

#endif
