#include <time.h>
#include "./constants.h"
#include "./vector.h"

struct Message{
    char mess[BUFFER_SIZE];
    bool received; // false: inviato e non ricevuto, true: inviato e ricevuto
    time_t send_timestamp;
};

struct UserMessages{
    char sender[BUFFER_SIZE];
    vector message_list;
    vector to_read;
    int total;
    time_t last_timestamp;
};

extern void userMessagesInit(struct UserMessages *u);

struct StructMessage{
    char dest[BUFFER_SIZE];
    vector userMessagesList;
};

extern void structMessageInit(struct StructMessage *sm);

struct UsersLink{
    char sender[BUFFER_SIZE];
    char dest[BUFFER_SIZE];
};