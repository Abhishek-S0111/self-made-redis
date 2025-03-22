#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>  //errno comes from here (global var)
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

static int32_t read_full(int fd,char *buf, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, buf, n);
        if(rv <= 0){
            return -1;  //prolly some unexpected error or EOF
        }

        assert((ssize_t)rv <= n);
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

        assert((ssize_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }

    return 0;
}

const size_t k_max_msg = 4096;

static int32_t query(int fd, const char* text){
    uint32_t len = (uint32_t)strlen(text);
    if( len > k_max_msg){
        return -1;  //length of text cannot exceed limit
    }

    char wbuf[4 + k_max_msg];  //no +1 cox we dont wanna write the null symbol
    memcpy(wbuf, &len, 4);  //assume it to be little endian
    memcpy(&wbuf, text, len);
    if(int32_t err = write_all(fd, wbuf, 4 + len)){
        return err;
    }

    //4 bytes header
    char rbuf[4 + k_max_msg + 1];  //extra +1 fo null terminator || EOL symbol
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);

    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    memcpy(&len, rbuf, 4); //assume little endian
    if(len > k_max_msg){
        msg("Too Long!!");
        return -1;
    }

    //reply body || handle replies
    err = read_full(fd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return -1;
    }

    //do something || some task done by the client
    printf("Server says: %.*s\n", len, &rbuf[4]);
    return 0;

}



int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0){
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  //refering to localhost 127.0.0.1

    int rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr));
    if(rv){
        die("connect");
    }

    char msg[] = "Hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if(n < 0){
        die("read");
    }

    printf("Server says: %s\n", rbuf);
    close(fd);

    return 0;
}