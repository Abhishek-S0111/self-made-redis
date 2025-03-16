#include<sys/socket.h>>


int main(int argv, char * argc[]){
    //first, obtain a socket ID
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    /*
        AF_INET     :  for IPv4
        SOCK_STREAM :  for TCP   
    */

    

}