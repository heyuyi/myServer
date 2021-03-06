//
// Created by heyuyi on 6/28/16.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "include/web_serve.h"
#include "include/io_func.h"

typedef struct {
    char method[16];
    char filepath[1024];
    char cgiargs[1024];
    char conttype[64];
    char contlen[16];
} header_t;

/*
 * Initialize header_t structure.
 */
void header_init(header_t *header)
{
    header->method[0]   = '\0';
    header->filepath[0] = '\0';
    header->cgiargs[0]  = '\0';
    header->conttype[0] = '\0';
    header->contlen[0]  = '\0';
}

/*
 * Get file and cgiargs from uri.
 */
int web_uri(const char *uri, char *file, char *cgiargs)
{
    char *p = strchr(uri, '?');
    if (p) {
        strcpy(cgiargs, p+1);
        *p = '\0';
        strcpy(file, ".");
        strcat(file, uri);
        return 1;
    } else {
        strcpy(cgiargs, "");
        strcpy(file, ".");
        strcat(file, uri);
        if (uri[strlen(uri)-1] == '/')
            strcat(file, "home.html");
        return 0;
    }
}

/*
 * Read the request header.
 * Record the variables in the header_t structure.
 */
int web_header(iobuf_t *iobuf, header_t *header)
{
    char tempbuf[TEMP_SIZE];
    char envp[64], envq[64];
    int ret = 0;
    for (;;) {
        readbline(iobuf, tempbuf, TEMP_SIZE);
        if (!strcmp(tempbuf, "\r\n"))
            return ret;
        sscanf(tempbuf, "%[^:]: %s\r\n", envp, envq);
        //printf("%s,%d,%s,%d\n", envp, strlen(envp), envq, strlen(envq));
        if (!strcmp(envp, "Content-Type"))
            strcpy(header->conttype, envq);
        else if (!strcmp(envp, "Content-Length")) {
            strcpy(header->contlen, envq);
            ret = atoi(envq);
        }
    }
}

/*
 * Return web error to the client.
 */
void web_error(int fd, const char *status, const char *msg)
{
    char databuf[MAXBUF_SIZE], headbuf[LINE_SIZE];
    sprintf(databuf, "<html><title>Server Error</title>");
    sprintf(databuf, "%s<body bgcolor=""ffffff"">\r\n", databuf);
    sprintf(databuf, "%s%s\r\n", databuf, status);
    sprintf(databuf, "%s<p>%s</p>\r\n", databuf, msg);
    sprintf(databuf, "%s<hr><em>myServer powered by heyuyi</em>\r\n", databuf);

    sprintf(headbuf, WEB_HANDER, status, "text/html", strlen(databuf));
    writen(fd, headbuf, strlen(headbuf));
    writen(fd, databuf, strlen(databuf));
}

/*
 * Response a static file to the client.
 */
void web_file(int fd, const char *file, int size)
{
    char headbuf[LINE_SIZE];
    char type[32];
    char *databuf;
    int ffd;
    if (strstr(file, ".html"))
        strcpy(type, "text/html");
    else if (strstr(file, ".gif"))
        strcpy(type, "image/gif");
    else if (strstr(file, ".jpg"))
        strcpy(type, "image/jpeg");
    else
        strcpy(type, "text/plain");

    sprintf(headbuf, WEB_HANDER, "200 OK", type, size);
    writen(fd, headbuf, strlen(headbuf));

    ffd = open(file, O_RDONLY, 0);
    databuf = mmap(0, size, PROT_READ, MAP_PRIVATE, ffd, 0);
    writen(fd, databuf, size);
    munmap(databuf, size);
    close(ffd);
}

/*
 * Set the environment variables in the forked child process,
 * before it exec the application.
 */
void web_setenv(header_t *header)
{
    if (header->method[0])
        setenv(REQUEST_METHOD, header->method, 1);
    if (header->cgiargs[0])
        setenv(QUERY_STRING, header->cgiargs, 1);
    if (header->conttype[0])
        setenv(CONTENT_TYPE, header->conttype, 1);
    if (header->contlen[0])
        setenv(CONTENT_LENGTH, header->contlen, 1);
}

/*
 * Exec a dynamic application in the GET request.
 * Redirect the STDOUT. The application do its own job.
 */
void web_exec_get(int fd, header_t *header)
{
    char arg0[LINE_SIZE];
    char *p;

    if (fork() == 0) {
        p = strrchr(header->filepath, '/');
        strcpy(arg0, p+1);
        web_setenv(header);
        dup2(fd, STDOUT_FILENO);
        execl(header->filepath, arg0, NULL);
    }
    wait(NULL);
}

/*
 * Exec a dynamic application in the POST request.
 * Redirect the STDOUT and STDIN. The application do its own job.
 * Using a pair of pipes between the two processes.
 */
void web_exec_post(int fd, int len, iobuf_t *iobuf, header_t *header)
{
    pid_t pid;
    int pipefd[2];
    int n = 0;
    char arg0[LINE_SIZE];
    char buf[MAXBUF_SIZE];
    char *p;

    if (pipe(pipefd) < 0)
        return;
    if ((pid = fork()) < 0) {
        return;
    } else if (pid == 0) {
        // Child process reads data from pipe, write response to socket.
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        dup2(fd, STDOUT_FILENO);

        // Set the environment variables and exec the application.
        p = strrchr(header->filepath, '/');
        strcpy(arg0, p+1);
        web_setenv(header);
        execl(header->filepath, arg0, NULL);
    } else {
        // Father process reads data from socket, write the data to the pipe.
        close(pipefd[0]);

        if (iobuf->cnt != 0) {
            writen(pipefd[1], iobuf->ptr, iobuf->cnt);
            len -= iobuf->cnt;
            iobuf->cnt = 0;
        }
        while (len > 0) {
            n = readn(fd, buf, len);
            writen(pipefd[1], buf, n);
            len -= n;
        }
        close(pipefd[1]);
        // Waiting for the child process.
        wait(NULL);
    }
}

/*
 * web service.
 * Analyse the header. Get the method, uri and other variables.
 * Invoke the application or return error.
 */
void web_serve(int fd)
{
    header_t header;
    iobuf_t iobuf;
    char tempbuf[TEMP_SIZE];
    char uri[2048];
    char version[16];
    struct stat sbuf;
    int tmp, len;

    // Read header.
    header_init(&header);
    iobuf_init(&iobuf, fd);
    readbline(&iobuf, tempbuf, TEMP_SIZE);
    if (3 != sscanf(tempbuf, "%s %s %s\r\n", header.method, uri, version))
        return;
    if (strcmp(version, "HTTP/1.1") && strcmp(version, "HTTP/1.0"))
        return;
    tmp = web_uri(uri, header.filepath, header.cgiargs);

    len = web_header(&iobuf, &header);
    if (stat(header.filepath, &sbuf) < 0) {
        sprintf(tempbuf, "myServer can not find the file: %s", header.filepath);
        web_error(fd, "404 Not found", tempbuf);
        return;
    }

    if (!strcmp(header.method, "GET")) {
        // GET method
        if (tmp) {
            if (S_ISREG(sbuf.st_mode) && (S_IXUSR & sbuf.st_mode)) {
                web_exec_get(fd, &header);
            } else {
                sprintf(tempbuf, "myServer can not run the program: %s", header.filepath);
                web_error(fd, "403 Forbidden", tempbuf);
                return;
            }
        } else {
            if (S_ISREG(sbuf.st_mode) && (S_IRUSR & sbuf.st_mode)) {
                web_file(fd, header.filepath, sbuf.st_size);
            } else {
                sprintf(tempbuf, "myServer can not read the file: %s", header.filepath);
                web_error(fd, "403 Forbidden", tempbuf);
                return;
            }
        }
    } else if (!strcmp(header.method, "POST")) {
        // POST method
        if (len == 0) {
            sprintf(tempbuf, "myServer can not serve the request: %s = 0", CONTENT_LENGTH);
            web_error(fd, "400 Bad Request", tempbuf);
            return;
        }
        if (S_ISREG(sbuf.st_mode) && (S_IXUSR & sbuf.st_mode)) {
            web_exec_post(fd, len, &iobuf, &header);
        } else {
            sprintf(tempbuf, "myServer can not run the program: %s", header.filepath);
            web_error(fd, "403 Forbidden", tempbuf);
            return;
        }
    } else {
        sprintf(tempbuf, "myServer does not implement the method: %s", header.method);
        web_error(fd, "501 Not Implemented", tempbuf);
        return;
    }
}