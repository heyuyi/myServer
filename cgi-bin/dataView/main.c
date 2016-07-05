#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#include "../../source/include/web_serve.h"
#include "../../source/include/io_func.h"

#define CMD     "SELECT ts, name, long_f, longitude, lat_f, latitude "\
                "FROM users, datas "\
                "WHERE users.userID = datas.userID "\
                "ORDER BY ts DESC "\
                "LIMIT %s"

int main(int argc, char *argv[]) {
    char databuf[MAXBUF_SIZE], headbuf[LINE_SIZE];
    char buf[8];
    char *cgiargs;
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    int len, rows;

    cgiargs = getenv(QUERY_STRING);
    if (cgiargs) {
        len = atoi(cgiargs);
        if (len < 10)
            strcpy(buf, "10");
        else
            sprintf(buf, "%d", len);
    } else
        strcpy(buf, "10");

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "heyuyi", "myServer", 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    sprintf(databuf, CMD, buf);
    if (mysql_query(conn, databuf)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_store_result(conn);
    rows = mysql_num_rows(res);
    if (mysql_num_fields(res) != 6)
        exit(2);
    sprintf(databuf, "<html><title>Data view</title>");
    sprintf(databuf, "%s<body bgcolor=""ffffff"">\r\n", databuf);
    sprintf(databuf, "%sThe lastest %d groups of data in the database:\r\n", databuf, rows);
    while ((row = mysql_fetch_row(res)) != NULL)
        sprintf(databuf, "%s<p>%s %10s %5s %s %5s %s</p>\r\n", databuf, row[0], row[1], row[2], row[3], row[4], row[5]);
    sprintf(databuf, "%s<hr><em>myServer powered by heyuyi</em>\r\n", databuf);

    sprintf(headbuf, WEB_HANDER, "200 OK", "text/html", strlen(databuf));
    writen(STDOUT_FILENO, headbuf, strlen(headbuf));
    writen(STDOUT_FILENO, databuf, strlen(databuf));

    mysql_free_result(res);
    mysql_close(conn);
    exit(0);
}
