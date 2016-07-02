//
// Created by heyuyi on 6/28/16.
//

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>

#include "include/io_func.h"

void transmit(int fd1, int fd2)
{
    char buf[4096];
    int n;

    for (;;) {
        n = read(fd1, buf, 4096);
        if (n > 0)
            writen(fd2, buf, n);
        else if (n == 0)
            break;
        else if (errno == EINTR)
            continue;
        else
            fprintf(stderr, "read error!");
    }
}

void transmit2(int fd1, int fd2)
{
    char buf[4096];
    fd_set rset;
    int maxfd = ((fd1 > fd2)? fd1:fd2) + 1;
    int n;

    FD_ZERO(&rset);
    for (;;) {
        FD_SET(fd1, &rset);
        FD_SET(fd2, &rset);
        select(maxfd, &rset, NULL, NULL, NULL);

        if (FD_ISSET(fd1, &rset)) {
            n = read(fd1, buf, 4096);
            if (n > 0)
                writen(fd2, buf, n);
            else if (n == 0)
                break;
            else if (errno == EINTR)
                continue;
            else
                fprintf(stderr, "read error!");
        }
        if (FD_ISSET(fd2, &rset)) {
            n = read(fd2, buf, 4096);
            if (n > 0)
                writen(fd1, buf, n);
            else if (n == 0)
                break;
            else if (errno == EINTR)
                continue;
            else
                fprintf(stderr, "read error!");
        }
    }
}