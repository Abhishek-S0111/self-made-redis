#include<sys/socket.h>>


int main(int argv, char * argc[]){
    //first, obtain a socket ID
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    /*
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

}