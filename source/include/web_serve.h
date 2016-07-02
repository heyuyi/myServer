//
// Created by heyuyi on 6/28/16.
//

#ifndef MYSERVER_WEB_SERVE_H
#define MYSERVER_WEB_SERVE_H

#define WEB_HANDER      "HTTP/1.1 %s\r\n"\
                        "Server: myServer powered by heyuyi\r\n"\
                        "Content-Type: %s\r\n"\
                        "Content-Length: %d\r\n\r\n"

#define REQUEST_METHOD  "REQUEST_METHOD"
#define QUERY_STRING    "QUERY_STRING"
#define CONTENT_TYPE    "CONTENT_TYPE"
#define CONTENT_LENGTH  "CONTENT_LENGTH"

#define MAXBUF_SIZE     8192
#define TEMP_SIZE       2048
#define LINE_SIZE       1024

extern void web_serve(int listenfd);

#endif //MYSERVER_WEB_SERVE_H
