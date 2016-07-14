//
// Created by heyuyi on 6/28/16.
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "include/serve.h"
#include "include/web_serve.h"

#define PID_NUM     15

pid_t pool[PID_NUM];
pthread_mutex_t *mptr;
int listenfd;

static void sig_int(int signo);
static void mutex_init(void);
static void childp(int fd);

int main() {
    struct sockaddr_in servaddr;
    struct sigaction act;
    int i;
    printf("myServer is starting...\n");

    // open socket
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error\n");
        exit(1);
    }

    // socket binds address(port 8082)
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8082);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "bind error\n");
        exit(2);
    }

    // start listening socket
    if(listen(listenfd, 1024) < 0) {
        fprintf(stderr, "listen error\n");
        exit(3);
    }

    // initialize mutex
    mutex_init();

    // profork PID_NUM child processes
    for (i = 0; i < PID_NUM; ++i) {
        if((pool[i] = fork()) == 0)
            // invoke child process function
            childp(listenfd);
        else if(pool[i] < 0) {
            fprintf(stderr, "fork error\n");
            exit(4);
        }
    }

    // set SIGINT signal callback function
    act.sa_handler = sig_int;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        for (i = 0; i < PID_NUM; ++i)
            kill(pool[i], SIGTERM);
        while (wait(NULL) > 0);
        fprintf(stderr, "sigaction error\n");
        exit(5);
    }

    for (;;)
        pause();
}

/*
 * SIGINT signal callback function.
 * Close the listening socket and kill all child processes.
 */
static void sig_int(int signo)
{
    pid_t pid;
    int i;
    close(listenfd);
    for (i = 0; i < PID_NUM; ++i)
        kill(pool[i], SIGTERM);
    while ((pid = wait(NULL))  > 0);
    if (errno != ECHILD) {
        fprintf(stderr, "wait error\n");
        exit(10);
    }
    printf("myServer has stoped.\n");
    exit(0);
}

/*
 * Initialize a mutex shared by all the child processes.
 * It's used to ensure that only child process is blocked on the accept function.
 */
static void mutex_init(void)
{
    pthread_mutexattr_t mattr;
    if ((mptr = (pthread_mutex_t *)mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        fprintf(stderr, "mmap error\n");
        exit(6);
    }
    if (pthread_mutexattr_init(&mattr) != 0) {
        fprintf(stderr, "pthread_mutexattr_init error\n");
        exit(6);
    }
    if (pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED) != 0) {
        fprintf(stderr, "pthread_mutexattr_setpshared error\n");
        exit(6);
    }
    if (pthread_mutex_init(mptr, &mattr) != 0) {
        fprintf(stderr, "pthread_mutex_init error\n");
        exit(6);
    }
    if (pthread_mutexattr_destroy(&mattr) != 0) {
        fprintf(stderr, "pthread_mutexattr_destroy error\n");
        exit(6);
    }
}

/*
 * Child process function.
 * Accept a socket from client request, which need get the mutex first.
 * Then start the web service.
 */
static void childp(int fd)
{
    struct sockaddr_in cliaddr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    int connfd;
    for (;;) {
        pthread_mutex_lock(mptr);
        if((connfd = accept(fd, (struct sockaddr *)&cliaddr, &socklen)) < 0) {
            fprintf(stderr, "accept error\n");
            continue;
        }
        pthread_mutex_unlock(mptr);
        printf("PID(%d): Client(%s:%d) has accepted...\n", getpid(), inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        //transmit2(connfd, STDOUT_FILENO);
        // web service
        web_serve(connfd);
        close(connfd);
        printf("PID(%d): Client(%s:%d) has disconnected.\n", getpid(), inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
    }
}