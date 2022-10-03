#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "./server/include/constants.h"
#include "./server/include/vector.h"

int listener, ret, addrlen, command, len, sd, i, port;
uint16_t lmsg, s_command;
    
fd_set master; 
fd_set read_fds; 
int fdmax; 

struct sockaddr_in my_addr, cl_addr;
char buffer[BUFFER_SIZE];

vector userRegister;