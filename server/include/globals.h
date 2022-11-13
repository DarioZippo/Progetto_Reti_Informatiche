#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef EXTERN 
#define EXTERN extern 
#endif

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "./constants.h"
#include "./vector.h"
#include "./record.h"

EXTERN int ret, addrlen, len, current_s, port, command;
EXTERN uint16_t lmsg, s_command;
    
EXTERN fd_set master; 
EXTERN fd_set read_fds; 
EXTERN int fdmax; 

EXTERN struct sockaddr_in my_addr, cl_addr;

EXTERN struct UserRegister userRegister;

EXTERN vector messages;
EXTERN vector usersLink;

#endif