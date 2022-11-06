#ifndef RECORD_H
#define RECORD_H

#include <time.h>
#include "./vector.h"

struct Record{
    char username[BUFFER_SIZE];
    int port;
    time_t login;
    time_t logout;
    // variabili extra rispetto al record registro delle specifiche
    int socket;
};

struct UserRegister{
    vector records;
    int onlineCounter;
};

extern void userRegisterInit(struct UserRegister *u);
#endif