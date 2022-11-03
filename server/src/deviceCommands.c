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
#include "./../include/globals.h"

void execDeviceCommand(int command){
    switch (command)
    {
    case 11:
        showOnlineUsers();
        break;
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
        hanging();
        break;
    case 4:
        show();
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
    file_user = fopen("./server/documents/user.txt", "a");
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
    int port;
    bool found;

    printf("IN\n");
    readCredentials(username, password);
    
    ret = recv(current_s, (void*)&port, sizeof(int), 0);
    if(ret < 0){
        printf("Errore nella ricezione\n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }

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
        insertLoggedUser(username, port);
        sendNotification(username);
    }
}

// funzione che implementa hanging
void hanging(){
    char dest[1024], num_mess_string[5];
    char message_info[1055]; //username(1024) num. mess.(5) timestamp(24)
    bool found = false;

    printf("HANGING\n");
    ret = recv(current_s, (void*)&dest, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }

    // cerco mittente nella lista dei mittenti
    struct StructMessage* temp_m;
    
    for(int i = 0; i < messages.pfVectorTotal(&messages); i++){
        temp_m = (struct StructMessage*)messages.pfVectorGet(&messages, i);
        //printf("Comp: %s con %s\n", temp_m->dest, dest);
        if(strcmp(temp_m->dest, dest) == 0){
            found = true;
            break;   
        }
    }

    // se non c'è significa che non ci sono messaggi
    if(found == false){
        // mando 0 che il codice che non ci sono messaggi
        ret = send(current_s, "ZERO", 4, 0);
        if(ret < 0)
            perror("Errore in fase di invio comando: \n");
                   
        return;
    }
    
    // scorro tutta la lista dei mittenti
    // nella struttura dedicata ai messaggi di uno specifico mittente mi salvo tutte le info
    // che servono per la hanging in modo che ogni volta non devo scorrere la lista dei messaggi
    vector *temp_v = &temp_m->userMessagesList;
    struct UserMessages* temp_u;

    printf("%d\n", temp_v->pfVectorTotal(temp_v));
    for(int j = 0; j < temp_v->pfVectorTotal(temp_v); j++){
        //printf("%d ", j);
        temp_u = (struct UserMessages*)temp_v->pfVectorGet(temp_v, j);
        if(temp_u->total > 0){
            strcpy(message_info, temp_u->sender);
            strcat(message_info, " ");
            sprintf(num_mess_string, "%d", temp_u->total);
            strcat(message_info, num_mess_string);   
            strcat(message_info, " ");
            strcat(message_info, ctime(&temp_u->last_timestamp));
            printf("%s %d\n", message_info, (int) strlen(message_info));
            ret = send(current_s, message_info, 1055, 0);
            if(ret < 0){
                perror("Errore in fase di invio messaggio: \n");            
                return;
            }
        }
    }

    // mando "ZERO" che è anche il codice che significa che non devo ricevere più niente dalla hanging
    ret = send(current_s, "ZERO", 4, 0);  
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");            
        return;
    }  
}

// funzione che implementa show
void show(){
    char sender[1024], dest[1024];
    bool found;

    struct UsersLink* temp_ul;

    printf("SHOW\n");

    // se l'utente ha digitato show x
    // ricevo lunghezza di x e poi x
    ret = recv(current_s, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    len = ntohs(lmsg);

    ret = recv(current_s, (void*)sender, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    
    // ricevo lunghezza dell'username che esegue show
    ret = recv(current_s, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    len = ntohs(lmsg);
    // ricevo username
    ret = recv(current_s, (void*)dest, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    //user[strlen(user)-1] = '\0';

    // Aggiorno struttura dati sulle show che sono state eseguite
    // Inserisco in testa alla lista la struct_per_show contenente le info sulla nuova show
    temp_ul = (void*) malloc(sizeof(struct UsersLink));
    strcpy(temp_ul->sender, sender);
    strcpy(temp_ul->dest, dest);
    usersLink.pfVectorAdd(&usersLink, temp_ul);

    // cerco destinatario (chi ha eseguito show) nella lista dei destinatari dei messaggi pendenti
    struct StructMessage* temp_d;
    
    for(int i = 0; i < messages.pfVectorTotal(&messages); i++){
        temp_d = (struct StructMessage*)messages.pfVectorGet(&messages, i);
        //printf("Comp: %s con %s\n", temp_m->dest, dest);
        if(strcmp(temp_d->dest, dest) == 0){
            found = true;
            break;   
        }
    }

    // se non c'è invio 0 che è il codice che indica che non devo ricevere più niente dalla show
    if(found == false){
        lmsg = htons(0);
        ret = send(current_s, &lmsg, sizeof(uint16_t), 0);
        if(ret < 0)
            perror("Errore in fase di invio comando: \n");
        return;
    }
    found = false;
    // cerco il mittente (x nell'esempio di prima)
    vector *temp_v = &temp_d->userMessagesList;
    struct UserMessages* temp_s;

    for(int i = 0; i < temp_v->pfVectorTotal(temp_v); i++){
        //printf("%d ", i);
        temp_s = (struct UserMessages*)temp_v->pfVectorGet(temp_v, i);
        //printf("%s\n%s\n", sender, temp_s->sender);
        len = strlen(sender);
        if(strncmp(temp_s->sender, sender, len) == 0){
            found = true;
            break;   
        }
    }

    // se non c'è invio 0
    if(found == false){
        lmsg = htons(0);
        ret = send(current_s, &lmsg, sizeof(uint16_t), 0);
        if(ret < 0)
            perror("Errore in fase di invio comando: \n");
        return;
    }
    
    // invio numero dei messaggi che sta per ricevere
    len = temp_s->total;
    lmsg = htons(len);
    ret = send(current_s, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore invio\n");
        return;
    }

    // invio tutti i messaggi non letti
    vector *temp_tr;
    struct Message *temp_m;

    //temp_tr = &temp_s->to_read;
    temp_tr = &temp_s->message_list;
    for(int i = 0; i < temp_tr->pfVectorTotal(temp_tr); i++){
        temp_m = (struct Message*)temp_tr->pfVectorGet(temp_tr, i);
        ret = send(current_s, temp_m->mess, 1024, 0);
        if(ret < 0){
            printf("Errore invio\n");
            return;
        }
        temp_m->received = true;
    }
    
    temp_s->total = 0;
    //vector_init(&temp_s->message_list);
    vector_init(&temp_s->to_read);
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
    ret = recv(current_s, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    len = ntohs(lmsg);
    ret = recv(current_s, (void*)username, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    username[len - 1] = '\0';

    printf("%s con len: %d, %d\n", username, strlen(username), len);

    struct Record* temp;
    for(int i = 0; i < userRegister.records.pfVectorTotal(&userRegister.records); i++){
        temp = (struct Record*)userRegister.records.pfVectorGet(&userRegister.records, i);
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
    ret = send(current_s, (void*) &pp, sizeof(uint16_t), 0);
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
    ret = recv(current_s, (void*)&sender, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }    
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }

    // ricevo username del destinatario
    ret = recv(current_s, (void*)&dest, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }

    // ricevo messaggio
    ret = recv(current_s, (void*)&message, 1024, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }

    //printf("Ricerca destinatario %d\n", messages.pfVectorTotal(&messages));
    struct StructMessage* temp_sm;
    // cerco destinatario nella lista dei destinatari
    for(int i = 0; i < messages.pfVectorTotal(&messages); i++){
        temp_sm = (struct StructMessage*)messages.pfVectorGet(&messages, i);
        if(strcmp(temp_sm->dest, dest) == 0){
            found = true;
            break;   
        }
    }
    printf("post ricerca, fine ricerca: %d\n", found);
    // se non c'è creo nuovo elemento della lista
    if(found == false){
        messages.pfVectorAdd(&messages, malloc(sizeof(struct StructMessage)));
        printf("%d\n", messages.pfVectorTotal(&messages));
        temp_sm = (struct StructMessage*)messages.pfVectorGet(&messages, messages.pfVectorTotal(&messages) - 1);
        structMessageInit(temp_sm);
        strcpy(temp_sm->dest, dest);
    }

    fflush(stdout);
    // cerco mittente nella lista dei mittenti di quel destinatario
    found = false;
    vector *temp_v = &temp_sm->userMessagesList;
    struct UserMessages* temp_u;
    //printf("Ricerca mittente %d\n", temp_v->pfVectorTotal(temp_v));
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

void showOnlineUsers(){
    printf("INVIA UTENTI ONLINE\n");

    // invio il numero di utenti online
    uint16_t s_onlineCounter = htons(userRegister.onlineCounter);
    ret = send(current_s, (void*) &s_onlineCounter, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    struct Record* temp;
    // invio tutti gli utenti che sono online
    for(int i = 0; i < userRegister.records.pfVectorTotal(&userRegister.records); i++){
        //printf("%d\n", i);
        temp = (struct Record*)userRegister.records.pfVectorGet(&userRegister.records, i);
        //printf("Temp: %s con len: %d\n", temp->username, strlen(temp->username));
        if(temp->logout == (time_t) NULL){ // timestamp_logout == NULL significa che è online
            ret = send(current_s, temp->username, 1024, 0);
            if(ret < 0){
                printf("Errore invio\n");
                return;
            }
        }
    }
}