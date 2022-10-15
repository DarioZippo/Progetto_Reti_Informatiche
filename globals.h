#ifndef GLOBALS_H
#define GLOBALS_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
//PARTE DA MODIFICARE PERCHE' LE GRANDEZZE TE LE DEVE
//MANDARE IL CLIENT
#include "./server/include/vector.h"

extern int listener, ret, addrlen, command, len, sd, current_s, port;
extern uint16_t lmsg, s_command;
    
extern fd_set master; 
extern fd_set read_fds; 
extern int fdmax; 

extern struct sockaddr_in my_addr, cl_addr;
extern char buffer[BUFFER_SIZE];

extern vector userRegister;

extern vector messages;
extern vector usersLink;

#endif