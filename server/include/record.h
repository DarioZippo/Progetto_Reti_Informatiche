#include <time.h>

struct Record{
    char username[1024];
    int porta;
    time_t login;
    time_t logout;
    // variabili extra rispetto al record registro delle specifiche
    int socket;
};