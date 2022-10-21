#include "./../include/constants.h"
#include "./../include/vector.h"
#include "./../include/record.h"

void userRegisterInit(struct UserRegister *u){
    u->onlineCounter = 0;
    vector_init(&u->records);
}