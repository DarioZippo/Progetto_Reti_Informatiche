#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "./../include/constants.h"
#include "./../include/serverCommands.h"
#include "./../include/deviceCommands.h"
#include "./../include/record.h"
#include "./../include/util.h"
#include "./../include/vector.h"

void execDeviceCommand(int command){
    switch (command)
    {
    case 1:
        signup();
        break;
    case 2:
        in();
        break;
    default:
        break;
    }
}

// funzione per registrare nuovo utente
void signup(){
    char username[1024], password[1024];
    FILE* file_user;
    printf("SIGNUP\n");
    readCredentials(username, password);
    file_user = fopen("./user.txt", "a");
    if(file_user == NULL){
        printf("Errore nell'apertura del file\n");
        return;
    }
    printf("to inserto into file: %s %s\n", username, password);
    fprintf(file_user, "%s %s\n", username, password);
    
    fclose(file_user);
}

// funzione per l'accesso di un utente già registrato
void in(){
    char username[1024], password[1024], user_psw[2048];
    bool found;

    printf("IN\n");
    readCredentials(username, password);
    
    strcpy(user_psw, username);
    strcat(user_psw, " ");
    strcat(user_psw, password);
    // trova utente restituisce true se trova il client con quelle credenziali nel file 0 altrimenti
    found = searchUser(user_psw);
    // non l'ha trovato, non inserisco il login
    if(found == false){
        printf("Non sei ancora registrato\n");
        return;
    }
    else if(found == true){
        insertLoggedUser(username);
    }
}