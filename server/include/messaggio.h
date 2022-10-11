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

/*
struct struct_per_show{
    char mittente[1024];
    char destinatario[1024];
    struct struct_per_show* next;
};

struct struct_per_show* lista_show_fatte;
*/