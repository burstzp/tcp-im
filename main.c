#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <string.h>
#include <assert.h>

#include "strings.h"
#include "hashtable.h"
#include "file.h"
#include "filter.h"
#include "httputil.h"

#define PORT        1234
#define MAX_ON_LINE     3
#define MEM_SIZE    1024
#define MAX_BUCCKETS 1024
struct event_base* base;
struct sock_ev {
    struct event* read_ev;
    struct event* write_ev;
    char* buffer;
};

#define TALK_TIP "talk to: "

struct clients
{
    int sock;
};

#define LOGIN_TIP "login: "
#define OFFERLINE  "/tmp/"

struct clients client[MAX_ON_LINE];

static int clinet_nums = 0;

static HashTable *ht = NULL;

void release_sock_event(struct sock_ev* ev)
{
    event_del(ev->read_ev);
    free(ev->read_ev);
    free(ev->write_ev);
    free(ev->buffer);
    free(ev);
}

void on_write(int sock, short event, void* arg)
{
    char* buf = (char*)arg;
    char *sendto = substr_p(buf, buf, strrchr(buf, '#'));
    if (!sendto) {
        free(buf);
        return;
    }
    
    sendto = trim(sendto);
    Hval *hval;
    ssize_t result;
    printf("on-wirte-sock = %d\n", sock);
    if (0 == hash_find(ht, sendto, &hval)) {
        printf("sock = %d, username = %s\n", hval->sock, hval->username);
        result = send(hval->sock, buf, strlen(buf), 0);
        if (result < 0) {
            hash_del(ht, hval->username);
            close(hval->sock);
        }
    } else {
        buffer *bufd = buffer_init();
        BUFFER_APPEND_CONST_STRING(bufd, buf);
        BUFFER_APPEND_CONST_STRING(bufd, "\n");
        
        char tmp[256];
        size_t size = 256;
        str_rcat(tmp, OFFERLINE, size);
        str_lcat(tmp, sendto, size);
        str_rcat(tmp, sendto, size);
        
        file_put_contents(tmp, bufd->ptr, "ab");
        buffer_free(bufd);
    }
    
    free(sendto);
    free(buf);
}

int get_sock(char *data)
{

    Hval *hval;
    return hash_find(ht, data, &hval);
}

int login_server(int sock, char *buf)
{
    char *data = trim(buf);
    Hval *hval;
    
    if (0 == hash_find(ht, data, &hval)) {
        return -1;
    }

    hash_add(ht, data, make_hval(sock, 0, data));
    return 0;
}

int verfiy(int sock, char *buf)
{
    if (get_sock(buf) == 0) {
        return 0;
    }
    
    char *ep = strrchr(buf, '@');
    if (!ep) return -1;
    int login = login_server(sock, buf);
    return login;
}

void on_read(int sock, short event, void* arg)
{
    int size;
    struct sock_ev* ev = (struct sock_ev*)arg;
    ev->buffer = (char*)malloc(MEM_SIZE);
    bzero(ev->buffer, MEM_SIZE);
    
    size = recv(sock, ev->buffer, MEM_SIZE, 0);
    if (size == 0) {
        release_sock_event(ev);
        close(sock);
        return;
    } else if (!strpos(ev->buffer, "quit")) {
        release_sock_event(ev);
        close(sock);
        return;
    } else if (-1 == verfiy(sock, ev->buffer)) {
        if (sock)
            send(sock, LOGIN_TIP, strlen(LOGIN_TIP), 0);
        return;
    } else if(strrchr(ev->buffer, '@')) {
        char *sendto = trim(ev->buffer);
        char tmp[256];
        strcpy(tmp, OFFERLINE);
        strcat(tmp, sendto);
        strcat(tmp, ".offline");
        if (is_file(tmp) == 0) {
            FILE *fp = fopen(tmp, "rb");
            char line[4096];
            while (fread(line, 1, sizeof(line), fp)) {
                send(sock, line, sizeof(line), 0);
            }
            fclose(fp);
            
//            buffer *buf = file_get_contents(tmp);
//            send(sock, buf->ptr, strlen(buf->ptr), 0);
            unlink(tmp);
//            buffer_free(buf);
        }
    }
    
    event_set(ev->write_ev, sock, EV_WRITE, on_write, ev->buffer);
    event_base_set(base, ev->write_ev);
    event_add(ev->write_ev, NULL);
}

void on_accept(int sock, short event, void* arg)
{
    struct sockaddr_in cli_addr;
    int newfd, sin_size;
    struct sock_ev* ev = (struct sock_ev*)malloc(sizeof(struct sock_ev));
    ev->read_ev = (struct event*)malloc(sizeof(struct event));
    ev->write_ev = (struct event*)malloc(sizeof(struct event));
    sin_size = sizeof(struct sockaddr_in);
    newfd = accept(sock, (struct sockaddr*)&cli_addr, &sin_size);
    
    event_set(ev->read_ev, newfd, EV_READ|EV_PERSIST, on_read, ev);
    event_base_set(base, ev->read_ev);
    event_add(ev->read_ev, NULL);
}

int main(int argc, char* argv[])
{
//    char *str = malloc(sizeof(char) * 20);
//    bzero(str, 20);
//    strcpy(str, "yiguopan@126.com");
//    char buf[1024] = {0};
//    
//    strncpy(buf, "guopan@1ok",sizeof("guopan@1ok"));
//    trim_string_end(buf, "@");
//    printf("buf = %s\n", buf);
//    str_replace_c(buf, "gu", 'k');
//    str_lcat(buf, "/", sizeof(buf));
//    str_rcat(buf, "hello", sizeof(buf));
//    printf("buf = %s\n", buf);
//    
//    exit(0);
    
//    char buf[1024] = {0};
//    curl_init();
//    curl_get("http://api.behinders.com/", buf);
//    char *p =trim(buf);
//    curl_destroy();
//    
//    printf("data = %s,1\n", p);
//    exit(0);
    
    
    struct sockaddr_in my_addr;
    int sock;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
    listen(sock, MAX_ON_LINE);
    
    slabs_init(671088);
    ht = _create_hashtable(MAX_BUCCKETS);
    
    struct event listen_ev;
    base = event_base_new();
    event_set(&listen_ev, sock, EV_READ|EV_PERSIST, on_accept, NULL);
    event_base_set(base, &listen_ev);
    event_add(&listen_ev, NULL);
    event_base_dispatch(base);
    
    return 0;
}