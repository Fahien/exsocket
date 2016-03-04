#include "basic.h"

/*
 * Reads all data in the buffer and examines them one byte at a time
 */
ssize_t exso_read(int fd, char *ptr) {
    static int read_cnt = 0;
    static char read_buf[MAXLINE];
    static char *read_ptr;

    if (read_cnt <= 0) {
        while ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno != EINTR) {
                return -1;	
            }
        }
        if (read_cnt == 0) {
            return 0;
        }
        read_ptr = read_buf;
    }
	
    read_cnt--;
    *ptr = *read_ptr++;

    return 1;
}

/*
 * Reads a line
 */
ssize_t exso_readln(int fd, void *vptr, size_t maxlen) {
    size_t n;
    int rc;
    char c;
    char *ptr;

    ptr = vptr;

    for (n = 1; n < maxlen; n++) {
        if ((rc = exso_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;
            }
        } else {
            if (rc == 0) {
                if (n == 1) {
                    return 0;
                } else {
                    break;
                }
            } else {
                return -1;
            }
        }
    }

    *ptr = 0;
    return n;
}

/*
 * Writes n bytes
 */
ssize_t exso_writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return n;
}
