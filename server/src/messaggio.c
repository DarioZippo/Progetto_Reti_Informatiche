#include "./../include/constants.h"
#include "./../include/vector.h"
#include "./../include/messaggio.h"

void userMessagesInit(struct UserMessages *u){
    u->last_timestamp = (time_t)NULL;
    u->total = 0;
    vector_init(&u->message_list);
    vector_init(&u->to_read);
}

void structMessageInit(struct StructMessage *sm){
    vector_init(&sm->userMessagesList);
}