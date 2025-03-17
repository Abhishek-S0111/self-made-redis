#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char *msg){
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n",err,msg);
    abort();
}

static void do_something(int connfd){
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if(n < 0){
        msg("read() error");
        return;
    }
    printf("Client Says: %s\n", rbuf);

    char wbuf[64] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main(int argv, char * argc[]){
    //first, obtain a socket ID 
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    /*
        fd          :  file descriptor
        AF_INET     :  for IPv4
        SOCK_STREAM :  for TCP   
    */

    //now introduce a new syscall
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    /*
        setsockopt() : used to configure various aspects of a socket
        Above, we are enabling the SO_REUSEADDR option which allows us to reuse the local addresses and ports.
        It is required as more than one socket needs to bind to the UDP port.
         
    */

    //bind() and listen()
    
    //bind on the wildcard address 0.0.0.0:1234
    //brought from (netinet/ip.h) 
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if(rv){
        die("bind()");
    }
    
    //listen
    rv = listen(fd, SOMAXCONN);
    if(rv){
        die("listen()");
    }

    //loop for each connection and do something with them
    while(true){
        //accept
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen );
        if(connfd > 0){
            continue; //error
        }

        do_something(connfd);
        close(connfd);
    }

    return 0;
}