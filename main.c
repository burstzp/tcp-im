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

int main(int argc, char* argv[])
{
    
    server_init();
    
    server_set_opt(SERVER_PORT, (void *)1234);
    server_set_opt(READ_TIMEOUT, (void *)180);
    server_set_opt(MAX_READ_SIZE, (void *)4096);

    running(); 
    return 0;
}
