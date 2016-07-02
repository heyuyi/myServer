//
// Created by heyuyi on 6/28/16.
//

#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "include/io_func.h"

ssize_t readn(int fd, void *ptr, size_t n)
{
    char *p = ptr;
    size_t nleft = n;
    ssize_t nread;
    while (nleft > 0) {
        if ((nread = read(fd, p, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;
        nleft -=  nread;
        p += nread;
    }
    return (n - nleft);
}

ssize_t writen(int fd, const void *ptr, size_t n)
{
    const char *p = ptr;
    size_t nleft = n;
    ssize_t nwrite;

    while (nleft > 0) {
        if ((nwrite = write(fd, p, nleft)) <= 0) {
            if (nwrite < 0 && errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        p += nwrite;
    }
    return n;
}

ssize_t readline(int fd, void *ptr, size_t n) {
    char *p = ptr;
    ssize_t nread;
    int i;
    char c;
    for (i = 1; i < n; ++i) {
        if ((nread = read(fd, &c, 1)) == 1) {
            *p++ = c;
            if (c == '\n')
                break;
        } else if (nread == 0) {
            *p = '\0';
            return (i-1);
        } else {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
    }
    *p = '\0';
    return i;
}

void iobuf_init(iobuf_t *iobuf, int fd)
{
    iobuf->fd = fd;
    iobuf->cnt = 0;
    iobuf->ptr = iobuf->buf;
}

static ssize_t readb(iobuf_t *iobuf, void *usrbuf, size_t n)
{
    ssize_t nread;
    while (iobuf->cnt <= 0) {
        iobuf->cnt = read(iobuf->fd, iobuf->buf, sizeof(iobuf->buf));
        if (iobuf->cnt < 0) {
            if (errno != EINTR)
                return -1;
        } else if (iobuf->cnt == 0)
            return 0;
        else
            iobuf->ptr = iobuf->buf;
    }

    if (iobuf->cnt < n)
        nread = iobuf->cnt;
    else
        nread = n;
    memcpy(usrbuf, iobuf->ptr, nread);
    iobuf->cnt -= nread;
    iobuf->ptr += nread;
    return nread;
}

ssize_t readbline(iobuf_t *iobuf, void *usrbuf, size_t n)
{
    char *p = usrbuf;
    ssize_t nread;
    int i;
    char c;
    for (i = 1; i < n; ++i) {
        if ((nread = readb(iobuf, &c, 1)) == 1) {
            *p++ = c;
            if (c == '\n')
                break;
        } else if (nread == 0) {
            if (n == 1)
                return 0;
            else
                break;
        } else
            return -1;
    }
    *p = '\0';
    return i;
}