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


typedef struct server
{
    int port;
    int fd;
    int timeout;
    int max_read_size;
} server_t;

static server_t *server;
static struct event_base *base;

typedef struct context {
    struct timeval timeout;
    request_t *request;
    response_t *response;
    struct event *read_ev;
    struct event *write_ev;
} context_t;

typedef enum server_opt {
    READ_TIMEOUT,
    SERVER_PORT,
    MAX_READ_SIZE,
    PID_FILE,
    LOG_FILE,
    LOG_LEVEL
} server_opt_t;

static int set_non_blocking(int fd);
static ulong inline get_microsec();
static void ev_on_accept(int fd, short ev, void *arg);
static void ev_on_read(int fd, short ev, void *arg);
static void ev_on_write(int fd, short ev, void *arg);
void server_init();
static context_t *context_init();
int server_set_opt(server_opt_t opt, void *);

void running();
#endif /* defined(__pushim__server__) */
