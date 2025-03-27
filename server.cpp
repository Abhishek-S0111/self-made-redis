#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <iostream>

using namespace std;

static void msg(const char *msg){
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n",err,msg);
    abort();
}

const size_t k_max_msg = 4096;


//uneeded for second step

// static void do_something(int connfd){
//     char rbuf[64] = {};
//     ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
//     if(n < 0){
//         msg("read() error");
//         return;
//     }
//     fprintf(stderr, "Client Says: %s\n", rbuf);

//     char wbuf[] = "world";
//     write(connfd, wbuf, strlen(wbuf));
// }

static int32_t read_full(int fd,char *buf, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, buf, n);
        if(rv <= 0){
            return -1;  //prolly some unexpected error or EOF
        }

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }

    return 0;     
}

static int32_t write_all(int fd, const char *buf, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buf, n);
        if(rv <= 0){
            return -1;
        }

        //To the me in the future, if you read this , I'm warning you once again to always check the typecasts in every type of code you write. Now and in the future. That Is All. THANK YOU
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }

    return 0;
}


static int32_t one_request(int connfd){
    //4 bytes header 
    char rbuf[4 + k_max_msg ];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);

    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4); //take assumption of little endian
    if(len > k_max_msg){
        msg("Meow");
        //cout << (char)len << endl;
        return -1;
    }

    //request body
    err = read_full(connfd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    //some functionality to check if working
    //eread client message
    //below line was removed as it was unnecessary
    //rbuf[4 + len] = '\0';
    fprintf(stderr,"Client Says: %.*s\n",len, &rbuf[4]);

    //reply to client message through the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);

    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);

    return write_all(connfd, wbuf, 4 + len);
}

int main(){
    //first, obtain a socket ID 
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        die("socket()");
    }
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
        if(connfd < 0){
            continue; //error
        }

        //only serve one client connection at a time
        while(true){
            int32_t err = one_request(connfd);
            if(err){
                break;
            }
        }
        
        close(connfd);
    }

    return 0;
}