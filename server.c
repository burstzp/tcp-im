#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <string.h>
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
#include <fcntl.h>  
#include <assert.h>

#include "strings.h"
#include "hashtable.h"
#include "file.h"
#include "filter.h"
#include "httputil.h"

#include "request.h"
#include "response.h"
#include "server.h"


#define M_SUCCESS 1
#define M_FAILURE 0

#define PORT        1234
#define MAX_ON_LINE     3
#define MEM_SIZE    1024
#define MAX_BUCCKETS 1024

static int set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return M_FAILURE;
    }
    return M_SUCCESS;
}

static void release_sock_event(context_t* ctx)
{
    event_del(ctx->read_ev);
    request_free(ctx->request);
    response_free(ctx->response);
    free(ctx);
}

static void ev_on_write(int sock, short event, void* arg)
{
    context_t *ctx = (context_t *)arg;
    response_t *response = ctx->response;

    printf("response-size = %d\n", response->size);
    int bytes_sent;
    do {
        if ((bytes_sent = send(sock, response->body, response->size, 0)) > 0) {
        
        }
    } while (bytes_sent == -1 && errno == EINTR);

    free(response->body);
    response->body = NULL;

}

static void ev_on_read(int sock, short event, void* arg)
{
    int size;
    context_t *ctx = (context_t *)arg;
    
    if (event == EV_TIMEOUT) {
        release_sock_event(ctx);
        close(sock);
        return;
    }

    //request_t *request = ctx->request;
    //request->body = calloc(1, server->max_read_size);;
    response_t *response = ctx->response;
    response->body = calloc(1, server->max_read_size);
    size = recv(sock, response->body, server->max_read_size, 0);
    if (size == 0) {
        release_sock_event(ctx);
        close(sock);
        return;
    } 
    
    response->size = size; 

    event_set(ctx->write_ev, sock, EV_WRITE, ev_on_write, ctx);
    event_base_set(base, ctx->write_ev);
    event_add(ctx->write_ev, &ctx->timeout);
}

static context_t *context_init()
{
    context_t *ctx = calloc(1, sizeof(context_t) + sizeof(request_t) + sizeof(response_t) + sizeof(struct event) * 2);    
    if (!ctx) {
        return NULL;
    }

    ctx->request = (request_t *)((unsigned char *)ctx + sizeof(context_t));
    ctx->response = (response_t *)((unsigned char *)ctx->request + sizeof(request_t));
    ctx->read_ev = (struct event *)((unsigned char *)ctx->response + sizeof(response_t));
    ctx->write_ev = (struct event *)((unsigned char *)ctx->read_ev + sizeof(struct event));
    ctx->timeout.tv_sec = server->timeout;
    ctx->timeout.tv_usec = 0;
    return ctx;
}

static void ev_on_accept(int sock, short ev, void *arg)
{
    context_t *ctx = context_init();
    assert(ctx);

    struct sockaddr_in client_addr;
    int client_fd, sin_size;

    sin_size = sizeof(struct sockaddr_in);

    client_fd = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

    if (client_fd == -1) {
        return;
    }
    
    if (set_non_blocking(client_fd) == M_FAILURE) {
        close(client_fd);
        return;
    }

    event_set(ctx->read_ev, client_fd, EV_READ|EV_PERSIST, ev_on_read, ctx);
    event_base_set(base, ctx->read_ev);
    event_add(ctx->read_ev, &ctx->timeout);
}

void server_init()
{
    server = calloc(1, sizeof(*server));
    assert(server);
}

int server_set_opt(server_opt_t opt, void * val)
{
    switch (opt) {
        case SERVER_PORT:
            server->port = *((int *)&val);
            break;
        case READ_TIMEOUT:
            server->timeout = *((int *)&val);
            break;
        case MAX_READ_SIZE:
            server->max_read_size = *((int *)&val);
        default:
            return 0;
    }

    return 1;
}

static int listening()
{
    struct sockaddr_in my_addr;
    int sock;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Failed to create a sock '%s'", strerror(errno));
        return;
    }

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(server->port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Failed to bind socket '%s'", strerror(errno));
        close(sock);
        return M_FAILURE;
    }
    
    if (listen(sock, MAX_ON_LINE)) {
        fprintf(stderr, "Failed start listening '%s'", strerror(errno));
        close(sock);
        return M_FAILURE;
    }
    
    if (set_non_blocking(sock) == M_FAILURE) {
        close(sock);
        return M_FAILURE;
    }

    server->fd = sock;
    return M_SUCCESS;
}

void running()
{
    if (listening() == M_FAILURE) {
        fprintf(stderr, "Listening port error '%s'", strerror(errno));
        return;
    } 

    struct event listen_ev;
    base = event_base_new();
    event_set(&listen_ev, server->fd, EV_READ|EV_PERSIST, ev_on_accept, NULL);
    event_base_set(base, &listen_ev);
    event_add(&listen_ev, NULL);
    event_base_dispatch(base);
}

