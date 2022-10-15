#include <time.h>
#include "./constants.h"
#include "./vector.h"

struct Message{
    char mess[1024];
    bool received; // false: inviato e non ricevuto, true: inviato e ricevuto
    time_t send_timestamp;
};

struct UserMessages{
    char sender[1024];
    vector message_list;
    vector to_read;
    int total;
    time_t last_timestamp;
};

extern void userMessagesInit(struct UserMessages *u);

struct StructMessage{
    char dest[1024];
    vector userMessagesList;
};

extern void structMessageInit(struct StructMessage *sm);

struct UsersLink{
    char sender[1024];
    char dest[1024];
};