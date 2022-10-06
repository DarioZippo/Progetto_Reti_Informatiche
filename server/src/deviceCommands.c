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
#include "./../../globals.h"

void execDeviceCommand(int command){
    switch (command)
    {
    case 1:
        hanging();
        break;
    case 2:
        show();
        break;
    case 3:
        chat();
        break;
    default:
        break;
    }
}

void showDeviceMenu(){
    printf("***************************** SERVER STARTED *********************************\n"
        "Digita un comando:\n"
        "\n"
        "1: hanging\n"
        "2: show\n"
        "3: chat\n"
        "4: share\n"
        "5: out\n"
    );
}

int inputDeviceMenu(){
    int choice;
    int min = 1, max = 5;
    do{
        scanf("%d", &choice);
        //getchar(); //Risolve un bug tra scanf e fgets eseguiti in successione
    }while(choice < min || choice > max);
    printf("Input dato\n");
    return choice;
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

void hanging(){

}

// funzione per implementare chat
// riceve lunghezza username_destinatario
// riceve username_destinatario
// se è online invia la porta a cui si è connesso username_destinatario
// se è offline invia 0
void chat(){
    char username[1024];
    int p = 0; // porta rimane a 0 se non trovo l'username nella lista
    uint16_t pp;

    printf("CHAT\n");
    ret = recv(i, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        disconnessione_client(i);
        return;
    }
    len = ntohs(lmsg);
    ret = recv(i, (void*)username, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        disconnessione_client(i);
        return;
    }
    username[len] = '\0';

    struct Record* temp;
    for(int i = 0; i < userRegister.pfVectorTotal(&userRegister); i++){
        temp = (struct Record*)userRegister.pfVectorGet(&userRegister, i);
        if(strcmp(temp->username, username) == 0){
            if(temp->logout == (time_t) NULL) // timestamp_logout == NULL significa che è online
                p = temp->porta;
            break;
        }
    }

    pp = htons(p);
    ret = send(i, (void*) &pp, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }
}