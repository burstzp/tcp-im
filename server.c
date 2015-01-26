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

int ev_set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return 0;
    }
    return 1;
}

ulong inline get_microsec() /* {{{ */ {
    struct timeval tv;
    if (gettimeofday(&tv, (struct timezone *)NULL) == 0) {
        return (tv.tv_sec * 1000000) + tv.tv_usec;
    } else {
        return 0;
    }
}

void ev_accept(int fd, short ev, void *arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(struct sockaddr_in);
    
    
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
        return ;
    
    context *ctx;
    
    ctx = calloc(1, sizeof(context) + sizeof(request_t) + sizeof(response_t));
    assert(ctx);
    
    if (ev_set_non_blocking(fd)) {
        fprintf(stderr, "Setting non-block mode failed %s", strerror(errno));
        free(ctx);
        close(client_fd);
    }
    
    ctx->request = (request_t *)((unsigned char *)ctx + sizeof(request_t));
    ctx->response = (response_t *)((unsigned char *)ctx->request + sizeof(response_t));
    ctx->timeout.tv_sec = server->timeout;
    ctx->timeout.tv_usec = 0;
    
    event_set(&ctx->ev_read, client_fd, EV_READ|EV_PERSIST, ev_on_read, ctx);
    event_add(&ctx->ev_read, &ctx->timeout);
}

void ev_on_read(int fd, short ev, void *arg)
{
    context *ctx = (context *)arg;
    request_t *request = ctx->request;
    
    if (ev == EV_TIMEOUT) {
        
    } else {
        char buf[sizeof(headerß)];
        int read_bytes;
        do {
            read_bytes = recv(fd, buf, sizeof(buf), 0);
        } while (read_bytes == -1 && errno == EINTR);

    }
}


void ev_on_write(int fd, short ev, void *arg)
{

}





