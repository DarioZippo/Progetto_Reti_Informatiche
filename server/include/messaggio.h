#include <time.h>
#include "./constants.h"
#include "./vector.h"

typedef int bool;
#define true 1
#define false 0

struct Message{
    char* mess;
    bool received; // false: inviato e non ricevuto, true: inviato e ricevuto
    time_t send_timestamp;
};

struct UserMessages{
    char sender[1024];
    vector message_list;
    //vector coda_lista;
    vector to_read;
    int total;
    time_t last_timestamp;
};

/*
struct struct_messaggi{
    char destinatario[1024];
    struct messaggi_utente* testa_lista;
    struct struct_messaggi* next;
};

struct struct_messaggi* struttura_messaggi;

struct struct_per_show{
    char mittente[1024];
    char destinatario[1024];
    struct struct_per_show* next;
};

struct struct_per_show* lista_show_fatte;
*/