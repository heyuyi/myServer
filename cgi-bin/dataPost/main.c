#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../../source/include/web_serve.h"
#include "../../source/include/io_func.h"

int main() {
    char databuf[] = "Welcome to use myServer.\r\n"
                    "You have posted a record successfully.";
    char headbuf[LINE_SIZE];
    char buf[MAXBUF_SIZE];
    int len = atoi(getenv(CONTENT_LENGTH));
    int n = readn(STDIN_FILENO, buf, len);
    //writen(STDOUT_FILENO, buf, n);

    sprintf(headbuf, WEB_HANDER, "200 OK", "text/plain", strlen(databuf));
    writen(STDOUT_FILENO, headbuf, strlen(headbuf));
    writen(STDOUT_FILENO, databuf, strlen(databuf));
    return 0;
}