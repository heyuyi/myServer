//
// Created by heyuyi on 6/28/16.
//

#ifndef MYSERVER_IO_FUNC_H
#define MYSERVER_IO_FUNC_H

#define IOBUF_SIZE      8192

typedef struct {
    int fd;
    int cnt;
    char *ptr;
    char buf[IOBUF_SIZE];
}iobuf_t;

extern ssize_t readn(int fd, void *ptr, size_t n);

extern ssize_t writen(int fd, const void *ptr, size_t n);

extern ssize_t readline(int fd, void *ptr, size_t n);

extern void iobuf_init(iobuf_t *iobuf, int fd);

extern ssize_t readbline(iobuf_t *iobuf, void *usrbuf, size_t n);


#endif //MYSERVER_IO_FUNC_H
