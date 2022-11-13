#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./vector.h"

void chatStateInit();
void readCredentials(char* credentials);
void sendCredentials(char* credentials, int command);
void signup();
void in();
int accessMenu();
void deviceAccess();
void showDeviceMenu();
int inputDeviceMenu();
void hanging();
void show();
void chat();
void chatP2P(int new_sd, char* message);
void share(int sock, char* message);
void receiveSharedFile(int current_s);
void receiveChatInfo();
void groupChat(int new_sd, char* message);
void groupUpdate(int current_s);
void addGroupMember(int new_sd, char* message);
void readSentMessages(char* dest);
void updateSentMessages(char* dest);
void showOnlineUsers();

bool searchContact(char* user);
void sendMessageToServer(char* sender, char* dest, char* message);

void execUserCommand(char command);

void peerDisconnection(int sock);
void out();
