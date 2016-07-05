#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#include "../../source/include/web_serve.h"
#include "../../source/include/io_func.h"

#define CMD     "INSERT INTO datas(userID, long_f, longitude, lat_f, latitude) "\
                "VALUES('%s', '%s', %f, '%s', %f)"

void strrmsp(char *str, size_t n);

int main() {
    char databuf[MAXBUF_SIZE];
    char headbuf[LINE_SIZE];
    char *plen = getenv(CONTENT_LENGTH);
    int len;
    char userID[20];
    char long_f[10], lat_f[10];
    float longitude, latitude;
    MYSQL *conn;

    if (!plen)
        exit(1);
    len = atoi(plen);
    if (readn(STDIN_FILENO, databuf, len) != len)
        exit(1);
    strrmsp(databuf, len);
    sscanf(databuf, "data={%[^:]:[%[^,],%f,%[^,],%f]}", userID, long_f, &longitude, lat_f, &latitude);
//    fprintf(stderr, "%s\n%s\n%f\n%s\n%f\n", userID, long_f, longitude, lat_f, latitude);

    if (!(strcmp(long_f, "E") && strcmp(long_f, "W")) && !(strcmp(lat_f, "N") && strcmp(lat_f, "S"))) {
        conn = mysql_init(NULL);
        if (!mysql_real_connect(conn, "localhost", "root", "heyuyi", "myServer", 0, NULL, 0)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }

        sprintf(databuf, CMD, userID, long_f, longitude, lat_f, latitude);
//        fprintf(stderr, "%s\n", databuf);
        if (mysql_query(conn, databuf))
            strcpy(databuf, "Welcome to use myServer.\r\nThe record you posted is incorrect.");
        else
            strcpy(databuf, "Welcome to use myServer.\r\nYou have posted a record successfully.");
        mysql_close(conn);
    } else
        strcpy(databuf, "Welcome to use myServer.\r\nThe record you posted is incorrect.");

    sprintf(headbuf, WEB_HANDER, "200 OK", "text/plain", strlen(databuf));
    writen(STDOUT_FILENO, headbuf, strlen(headbuf));
    writen(STDOUT_FILENO, databuf, strlen(databuf));

    exit(0);
}

void strrmsp(char *str, size_t n)
{
    int i, j;
    for (i = 0, j = 0; i < n; ++i) {
        if(str[i] != ' ')
            str[j++] = str[i];
    }
    str[j] = '\0';
}