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

#include "./constants.h"
#include "./vector.h"
#include "./chat_state.h"

EXTERN int sd, ret, port, len, new_sd, server_port;
EXTERN char username[BUFFER_SIZE];
EXTERN uint16_t lmsg;
EXTERN struct sockaddr_in server_addr, my_addr, peer_addr;
EXTERN int username_len;
EXTERN bool logged;
EXTERN fd_set master; 
EXTERN fd_set read_fds;
EXTERN int fdmax, listener_sock;

EXTERN struct ChatState chatState;

#endif