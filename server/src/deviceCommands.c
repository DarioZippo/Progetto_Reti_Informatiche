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
#include "./../include/messaggio.h"
#include "./../../globals.h"

void execDeviceCommand(int command){
    switch (command)
    {
    case 10:
        pendentMessage();
        break;
    case 1:
        signup();
        break;
    case 2:
        in();
        break;
    case 3:
        //hanging();
        break;
    case 4:
        //show();
        break;
    case 5:
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
        clientDisconnection(i);
        return;
    }
    len = ntohs(lmsg);
    ret = recv(i, (void*)username, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(i);
        return;
    }
    username[len - 1] = '\0';

    //printf("%s con len: %d, %d\n", username, strlen(username), len);

    struct Record* temp;
    for(int i = 0; i < userRegister.pfVectorTotal(&userRegister); i++){
        temp = (struct Record*)userRegister.pfVectorGet(&userRegister, i);
        //printf("Temp: %s con len: %d\n", temp->username, strlen(temp->username));
        if(strcmp(temp->username, username) == 0){
            printf("Trovato nella lista\n");
            if(temp->logout == (time_t) NULL){ // timestamp_logout == NULL significa che è online
                printf("E' ONLINE!!!!\n");
                p = temp->port;
            }
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

// funzione che server per memorizzare messaggi inviati quando il destinatario è offline
void pendentMessage(){
    char sender[1024], dest[1024], message[1024];
    time_t rawtime;
    bool found = false;

    printf("PENDENTE\n");
    // ricevo username del mittente
    ret = recv(i, (void*)&sender, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }    
    if(ret == 0){
        clientDisconnection(i);
        return;
    }

    // ricevo username del destinatario
    ret = recv(i, (void*)&dest, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(i);
        return;
    }

    // ricevo messaggio
    ret = recv(i, (void*)&message, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(i);
        return;
    }

    printf("Ricerca destinatario %d\n", messages.pfVectorTotal(&messages));
    struct StructMessage* temp_m;
    // cerco destinatario nella lista dei destinatari
    for(int i = 0; i < messages.pfVectorTotal(&messages); i++){
        temp_m = (struct StructMessage*)messages.pfVectorGet(&messages, i);
        if(strcmp(temp_m->dest, dest) == 0){
            found = true;
            break;   
        }
    }
    printf("post ricerca, fine ricerca: %d\n", found);
    // se non c'è creo nuovo elemento della lista
    if(found == false){
        messages.pfVectorAdd(&messages, malloc(sizeof(struct StructMessage)));
        printf("%d\n", messages.pfVectorTotal(&messages));
        temp_m = (struct StructMessage*)messages.pfVectorGet(&messages, messages.pfVectorTotal(&messages) - 1);
        structMessageInit(temp_m);
    }

    fflush(stdout);
    // cerco mittente nella lista dei mittenti di quel destinatario
    found = false;
    vector *temp_v = &temp_m->userMessagesList;
    struct UserMessages* temp_u;
    printf("Ricerca mittente %d\n", temp_v->pfVectorTotal(temp_v));
    for(int i = 0; i < temp_v->pfVectorTotal(temp_v); i++){
        //printf("%d ", i);
        temp_u = (struct UserMessages*)temp_v->pfVectorGet(temp_v, i);
        if(strcmp(temp_u->sender, sender) == 0){
            found = true;
            break;   
        }
    }
    printf("\n post ricerca, fine ricerca: %d \n", found);
    // se non c'è creo nuovo elemento
    if(found == false){
        temp_v->pfVectorAdd(temp_v, malloc(sizeof(struct UserMessages)));
        printf("%d\n", temp_v->pfVectorTotal(temp_v));
        temp_u = (struct UserMessages*)temp_v->pfVectorGet(temp_v, temp_v->pfVectorTotal(temp_v) - 1);
        userMessagesInit(temp_u);
        strcpy(temp_u->sender, sender);
    }

    // inserisco messaggio in coda alla lista dei messaggi
    // per non scorrere ogni volta la lista mantengo un puntatore alla coda della lista
    struct Message *new_mess = (void*) malloc(sizeof(struct Message));
    printf("messaggio: %s\n", message);
    strcpy(new_mess->mess, message);
    new_mess->received = false;
    time(&rawtime);
    printf("INSERIMENTO: %s\n", ctime(&rawtime));
    new_mess->send_timestamp = rawtime;

    temp_u->message_list.pfVectorAdd(&temp_u->message_list, new_mess);
    temp_u->total++;
    temp_u->last_timestamp = rawtime;
    temp_u->to_read.pfVectorAdd(&temp_u->to_read, new_mess);
}