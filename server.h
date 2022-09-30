#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "vector.h"

//PARTE DA MODIFICARE PERCHE' LE GRANDEZZE TE LE DEVE
//MANDARE IL CLIENT
#define BUFFER_SIZE 1024
#define REQUEST_LEN 4 // REQ\0

typedef int bool;
#define true 1
#define false 0

int listener, ret, addrlen, command, len, sd, i;
uint16_t lmsg, s_command;
    
fd_set master; 
fd_set read_fds; 
int fdmax; 

struct sockaddr_in my_addr, cl_addr;
char buffer[BUFFER_SIZE];

struct Record{
    char username[1024];
    int porta;
    time_t login;
    time_t logout;
    // variabili extra rispetto al record registro delle specifiche
    int socket;
};

vector userRegister;

void signup();
void in();
void insertLoggedUser(char* username);
void writeLoginOnFile(char* username, char* record, int len, int porta, time_t rawtime);
bool searchUser(char* user_psw);
void read_credentials(char* username, char* password);
void client_disconnection(int sock);
void execCommand(int command);