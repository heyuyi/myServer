#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../source/include/web_serve.h"
#include "../../source/include/io_func.h"

int main(int argc, char *argv[]) {
    char databuf[MAXBUF_SIZE], headbuf[LINE_SIZE];
    char *cgiargs = getenv(QUERY_STRING);
    char defa[] = "10";
    int len = atoi(cgiargs);

    if (len == 0) {
        len = 10;
        cgiargs = defa;
    }
    sprintf(databuf, "<html><title>Data view</title>");
    sprintf(databuf, "%s<body bgcolor=""ffffff"">\r\n", databuf);
    sprintf(databuf, "%sThese are the lastest %s groups of data in the database:\r\n", databuf, cgiargs);
    sprintf(databuf, "%s<hr><em>myServer powered by heyuyi</em>\r\n", databuf);

    sprintf(headbuf, WEB_HANDER, "200 OK", "text/html", strlen(databuf));
    writen(STDOUT_FILENO, headbuf, strlen(headbuf));
    writen(STDOUT_FILENO, databuf, strlen(databuf));
    exit(0);
}