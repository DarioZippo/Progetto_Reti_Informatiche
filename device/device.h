#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define RESPONSE_LEN 9 // HH:MM:SS\0

typedef int bool;
#define true 1
#define false 0

int sd, ret, port, len, new_sd;
char command[1024], buffer[1024];
uint16_t lmsg;
struct sockaddr_in server_addr, my_addr, peer_addr;
char* username;
int username_len;
bool logged = false;
fd_set master; 
fd_set read_fds;
int fdmax, i, listener_sock;

void readCredentials(char* credentials);
void sendCredentials(char* credentials, int command);
void signup();
void in();
int accessMenu();
void deviceAccess();
void showDeviceMenu();
int inputDeviceMenu();
void execDeviceCommand(int command);
void hanging();
void show();
void chat();

void out();
