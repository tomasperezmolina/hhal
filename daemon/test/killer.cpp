#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h> 
#include <string.h> 

#include "cuda_commands.h"

#define SOCKET_PATH "/tmp/server-test"
   
int main(int argc, char const *argv[]) { 
    int sock = 0, valread; 
    struct sockaddr_un serv_addr; 

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sun_family = AF_UNIX; 
    strcpy(serv_addr.sun_path, SOCKET_PATH);
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    cuda_daemon::command_base_t cmd;
    cuda_daemon::init_end_command(cmd);

    send(sock, &cmd, sizeof(cmd), 0); 
    printf("Killing server\n"); 

    close(sock);
    
    return 0; 
} 
