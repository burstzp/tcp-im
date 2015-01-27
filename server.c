//
//  server.c
//  pushim
//
//  Created by 易国磐 on 15-1-26.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//
#include <stdarg.h> 	/* for va_list */
#include <stdio.h>   	/* for fprintf */
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  	/* for fork & setsid */
#include <time.h>  		/* for ctime */
#include <sys/types.h>
#include <sys/stat.h> 	/* for umask */
#include <sys/socket.h> /* for sockets */
#include <sys/un.h>  	/* for un */
#include <sys/wait.h>   /* for waitpid */
#include <sys/time.h>   /* for gettimeofday */
#include <netdb.h>  	/* for gethostbyname */
#include <pwd.h>        /* for getpwnam */
#include <grp.h>        /* for getgrnam */
#include <arpa/inet.h> 	/* for inet_ntop */
#include <netinet/in.h>

#include <assert.h>

#include "strings.h"
#include "hashtable.h"
#include "file.h"
#include "filter.h"
#include "httputil.h"
#include "server.h"
#include "request.h"
#include "response.h"

#include <fcntl.h> 		/* for fcntl */
static server_t *server;
struct event_base *base;

void server_init(char *hostname, int port, int maxonline, int timeout)
{
    server = calloc(1, sizeof(server_t)+sizeof(hostname));
    assert(server);
//    server->hostname = (char *)((unsigned char *)server + sizeof(hostname));
//    memcpy(server->hostname, hostname, sizeof(hostname));
    server->port = port;
    server->maxonline = maxonline;
    server->timeout = timeout;
}

void free_server()
{
    if (server) {
        free(server);
    }
}

static int set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return 0;
    }
    return 1;
}

static ulong inline get_microsec() /* {{{ */ {
    struct timeval tv;
    if (gettimeofday(&tv, (struct timezone *)NULL) == 0) {
        return (tv.tv_sec * 1000000) + tv.tv_usec;
    } else {
        return 0;
    }
}

static void ev_on_accept(int fd, short ev, void *arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(struct sockaddr_in);
    
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
        return ;
    
    context *ctx;
    
    ctx = calloc(1, sizeof(context) + sizeof(request_t));// + sizeof(response_t) + sizeof(struct event)*2);
    assert(ctx);
    
    if (!set_non_blocking(fd)) {
        fprintf(stderr, "Setting non-block mode failed %s", strerror(errno));
        free(ctx);
        close(client_fd);
        return;
    }
    
    ctx->request = (request_t *)((unsigned char *)ctx + sizeof(request_t));
    ctx->response = (response_t *)((unsigned char *)ctx->request + sizeof(response_t));
//    ctx->ev_read = (struct event *)((unsigned char *)ctx->response + sizeof(struct event));
//    ctx->ev_write = (struct event *)((unsigned char *)ctx->ev_write + sizeof(struct event));

    ctx->timeout.tv_sec = server->timeout;
    ctx->timeout.tv_usec = 0;
    
    event_set(&ctx->ev_read, client_fd, EV_READ|EV_PERSIST, ev_on_read, ctx);
    //event_base_set(base, &ctx->ev_read);
    event_add(&ctx->ev_read, &ctx->timeout);
}

static void ev_on_read(int fd, short ev, void *arg)
{
    context *ctx = (context *)arg;
    assert(ctx);
    request_t *request = ctx->request;
    
    if (ev == EV_TIMEOUT) {
        ev_close_connection(fd, ctx); 
    } else {
        char buf[12];
        strcpy(buf, "1024");
        int read_bytes;
        
        request->size = atoi(buf); 
        request->body = malloc(request->size);
        bzero(request->body, 0); 
        do {
            read_bytes = recv(fd, request->body, request->size, 0);
        } while (read_bytes == -1 && errno == EINTR);
        
        if (read_bytes <= 0 || read_bytes > request->size) {
            ev_close_connection(fd, ctx);
            fprintf(stderr, "Recv error failed size is smaller %s", strerror(errno));
            return; 
        }
        
        response_t *response = ctx->response;
        response->body = malloc(read_bytes);
        response->size = read_bytes;
        memcpy(response->body, request->body, request->size);
       
        event_set(&ctx->ev_write, fd, EV_WRITE|EV_PERSIST, ev_on_write, ctx);
        event_add(&ctx->ev_write, &ctx->timeout);
        event_del(&ctx->ev_read);
        return;
    }
}

static void ev_on_write(int fd, short ev, void *arg)
{
    context *ctx = (context *)arg;
    assert(ctx);
    response_t *response = ctx->response;
    if (ev == EV_TIMEOUT) {
        ev_close_connection(fd, ctx); 
    } else {
        int bytes_sent = 0;
        int ctx_sent = 0;
        do {
             if ((bytes_sent = send(fd, response->body, response->size, 0)) > 0) {
                 ctx_sent += bytes_sent;
            }
        } while (bytes_sent == -1 && errno == EINTR);
        
        if (ctx_sent < response->size) { 
            return;
        }
    } 
}

static void ev_close_connection(int fd, context *ctx) /* {{{ */ {
    close(fd);
    event_del(&ctx->ev_read);
    event_del(&ctx->ev_write);
    request_free(ctx->request);
    response_free(ctx->response);
    free(ctx);
}

static int listening()
{
    struct sockaddr sa;
    int sockfd = 0;
    
    assert(server);
    assert(server->port);
    
    struct sockaddr_in ia;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    memset(&ia, 0, sizeof(ia));

    ia.sin_family = AF_INET;
    ia.sin_port   = htons(server->port);
    ia.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd,  (struct sockaddr*)&ia, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "bind sock error, %s", strerror(errno));
        return 0;
    }
    
    if (listen(sockfd, server->maxonline)) {
        fprintf(stderr, "listen max on line error, %s", strerror(errno));
        return 0;
    }

    if (!set_non_blocking(sockfd)) {
        fprintf(stderr, "set sock error");
        return 0;
    }

    server->fd = sockfd;
    return 1;
}

void running()
{
    if (!listening()) {
        return;
    }

    struct event *ev_accept = (struct event*)malloc(sizeof(*ev_accept));
    event_init();
    //base = event_base_new();
    event_set(ev_accept, server->fd, EV_READ|EV_PERSIST, ev_on_accept, NULL);
    //event_base_set(base, ev_accept);
    event_add(ev_accept, NULL);
    event_dispatch();

    //event_base_dispatch(base);
}
