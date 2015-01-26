//
//  server.h
//  pushim
//
//  Created by 易国磐 on 15-1-26.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__server__
#define __pushim__server__

#include <stdio.h>

#include "event.h"
#include "request.h"
#include "response.h"

typedef unsigned long ulong;
typedef unsigned int  uint;

typedef struct context
{
    response_t *response;
    request_t *request;
    struct event ev_read;
    struct event ev_write;
    struct server *ctx_server;
    struct timeval timeout;
} context;

typedef struct server
{
    int fd;
    int timeout;
    ulong start_time;
    char *pid_file;
    char *log_file;
}server_t;

void ev_accept(int fd, short ev, void *arg);
void ev_on_read(int fd, short ev, void *arg);
void ev_on_write(int fd, short ev, void *arg);

#endif /* defined(__pushim__server__) */
