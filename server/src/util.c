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

void insertLoggedUser(char* username, int port){
    int len = strlen(username);
    char record[1500];
    time_t rawtime;

    // cerco utente nella lista degli utenti hanno fatto una connessione
    bool found = false;
    int i;
    
    struct Record* temp;
    for (i = 0; i < userRegister.records.pfVectorTotal(&userRegister.records); i++)
    {
        temp = (struct Record*)userRegister.records.pfVectorGet(&userRegister.records, i);
        if(strcmp(temp->username, username) == 0){
            found = true;
            break;
        }
    }
    
    // se non c'è creo nuovo elemento della lista
    if(found == false){
        userRegister.records.pfVectorAdd(&userRegister.records, malloc(sizeof(struct Record)));
        printf("%d\n", userRegister.records.pfVectorTotal(&userRegister.records));
        temp = (struct Record*)userRegister.records.pfVectorGet(&userRegister.records, userRegister.records.pfVectorTotal(&userRegister.records) - 1);
    }
    printf("Temp individuato\n");
    // aggiorno entry del server
    time(&rawtime);

    strcpy(temp->username, username);
    temp->port = port;
    temp->login = rawtime; // login a timestamp corrente
    temp->logout = (time_t) NULL; // logout NULL perchè è online
    temp->socket = current_s; // socket viene salvato per la disconnessione improvvisa
    writeLoginOnFile(username, record, len, 4242/*porta*/, rawtime);

    userRegister.onlineCounter++;
}

// scrivo login su file di log
void writeLoginOnFile(char* username, char* record, int len, int porta, time_t rawtime){
    printf("Scrittura su file\n");
    FILE* file_login = fopen("./login.txt", "a");
    if(file_login == NULL){
        printf("Errore nell'apertura del file\n");
        return;
    }
    
    time(&rawtime);
    fprintf(file_login, "%s %d %s %s\n", username, porta, ctime(&rawtime), "NULL");
    fclose(file_login);
}

// trova utente, utilizzata in fase di login per cercare utente registrato nel file user.txt
bool searchUser(char* user_psw){
    FILE* file_user;
    char* line, current_credentials;
    int read;
    bool found = false;

    file_user = fopen("./user.txt", "r");
    if(file_user == NULL){
        printf("Errore nell'apertura del file\n");
        return false;
    }

    while ((read = getline(&line, &len, file_user)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        line[read - 1] = '\0';
        //printf("%s\n", line);

        if(strcmp(line, user_psw) == 0){ 
            found = true;
            /*
            ret = send(i, "LOGIN\0", 6, 0);
            if(ret < 0){
                printf("Errore nell'invio\n");
                return 0;
            }
            */
            break;
        }
    }
    //printf("Dopo la lettura\n");
    // se non l'ha trovato, invia "NO" e il client rileva che il login ha fallito
    if(found == false){
        /*ret = send(i, "NO\0", 6, 0);
        if(ret < 0){
            printf("Errore nell'invio\n");
            return 0;
        }*/
    }
    fclose(file_user);
    printf("Fine ricerca: %d\n", found);
    return found;
}

// riceve credenziali, username e password
void readCredentials(char* username, char* password){
    // riceve lunghezza username
    ret = recv(current_s, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione LEN_US: \n");
        return;
    }
    len = ntohs(lmsg);
    printf("Lunghezza us: %d\n", len);
    // riceve username
    ret = recv(current_s, (void*)username, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione US: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    username[len] = '\0';
    // riceve lunghezza password
    ret = recv(current_s, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione LEN_P: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    len = ntohs(lmsg);
    printf("Lunghezza ps: %d\n", len);
    // riceve password
    ret = recv(current_s, (void*)password, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione P: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(current_s);
        return;
    }
    password[len] = '\0';

    printf("credentials: %s %s\n", username, password);
}

// funzione che chiude il socket di un client che si è disconnesso improvvisamente
// imposta anche il timestamp di logout uguale al timestamp corrente
// la funzione viene chiamata quando una recv restituisce 0
void clientDisconnection(int sock){
    close(sock); // chiudo socket
    FD_CLR(sock, &master);

    time_t rawtime;
    
    // cerco nella lista degli utenti il client con quel socket
    bool found = false;
    int i;
    
    struct Record* temp;
    for (i = 0; i < userRegister.records.pfVectorTotal(&userRegister.records); i++)
    {
        temp = (struct Record*)userRegister.records.pfVectorGet(&userRegister.records, i);
        if(temp->socket == sock){
            found = true;
            break;
        }
    }
    if(found == false)
        return;

    temp->logout = time(&rawtime); // imposto timestamp logout
    userRegister.onlineCounter--;
    
    perror("Disconnessione client \n");
    printf("CLIENT DISCONNESSO %s %s", temp->username, ctime(&temp->logout));
}

void restoreLogin(){
    FILE* file_user;
    char* line, *current_username;
    int read, i = 0;
    bool found = false;

    file_user = fopen("./user.txt", "r");
    if(file_user == NULL){
        printf("Errore nell'apertura del file\n");
        return;
    }

    while ((read = getline(&line, &len, file_user)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        line[read - 1] = '\0';
        printf("%s\n", line);
        
        while(1){
            if(line[i] == ' '){
                break;
            }
            i++;
        }
        current_username = (char *) malloc(i + 1);
        strncpy(current_username, line, i);
        current_username[i] = '\0';
        //insertLoggedUser(current_username, port);
        i = 0;
    }
    // se non l'ha trovato, invia "NO" e il client rileva che il login ha fallito
    if(found == false){
        /*ret = sendcurrent_s, "NO\0", 6, 0);
        if(ret < 0){
            printf("Errore nell'invio\n");
            return 0;
        }*/
    }
    fclose(file_user);
    printf("Fine ripristino:\n");
}

// inizializzo le strutture dati dedicate ai messaggi
// il file che leggo è saved_messages.txt che contiene tutti i messaggi pendenti
// il file viene scritto quando si fa esc
void restoreMessages(){
    FILE* saved_messages;
    char *line;
    int current_type = 0, num_mess, read; 
    bool primo_mittente = true, primo_non_ric = true, no_read = false;
    struct StructMessage* temp_sm;
    struct UserMessages* temp_um;
    vector* temp_v;
    struct Message *new_mess;

    saved_messages = fopen("./server/documents/saved_messages.txt", "r");
    if(saved_messages == NULL){
        printf("Errore nell'apertura del file\n");
        return false;
    }
    // il codice sottostante serve per ricreare le liste dei messaggi pendenti a partire dal file
    // sapendo che il file è strutturato in una particolare maniera
    // quando c'è un nuovo destinatario da inserire nella lista si ha
    // dest:
    // username_destinatario
    // quando c'è un nuovo mittente da inserire nella lista si ha
    // mitt:
    // username_mittente
    // dopo il mittente ci sono i messaggi, per ogni messaggio ci sono 3 righe
    // messaggio
    // 0 o 1 (0: non ricevuto, 1: ricevuto)
    // timestamp invio (salvato come short unsigned int)
    while(1){
        if(no_read == false){
            read = getline(&line, &len, saved_messages);
            if(read == -1)
                break;
        }
        no_read = false;
        line[read - 1] = '\0';
        if(strncmp(line, "dest:", 5) == 0)
            current_type = 0;
        if(strncmp(line, "mitt:", 5) == 0)
            current_type = 1;
        switch(current_type){
            //DEST
            case 0:{
                messages.pfVectorAdd(&messages, malloc(sizeof(struct StructMessage)));
                temp_sm = (struct StructMessage*)messages.pfVectorGet(&messages, messages.pfVectorTotal(&messages) - 1);
                structMessageInit(temp_sm);
                
                read = getline(&line, &len, saved_messages);
                if(read == -1)
                    break;

                line[read - 1] = '\0';
                strcpy(temp_sm->dest, line);
                temp_v = &temp_sm->userMessagesList;
                primo_mittente = true;
                break;
            }
            //SENDER
            case 1:{
                //Aggiungo la chat relativa al sender
                temp_v->pfVectorAdd(temp_v, malloc(sizeof(struct UserMessages)));
                temp_um = (struct UserMessages*)temp_v->pfVectorGet(temp_v, temp_v->pfVectorTotal(temp_v) - 1);
                userMessagesInit(temp_um);
                
                read = getline(&line, &len, saved_messages);
                if(read == -1)
                    break;

                line[read - 1] = '\0';
                strcpy(temp_um->sender, line);
                
                //Mi preparo a leggere il primo messaggio
                new_mess = (void*) malloc(sizeof(struct Message));
                new_mess->received = false;

                //Mi sposto nella fase di lettura messaggi
                current_type = 2;
                primo_non_ric = true;
                break;
            }
            //LETTURA MESSAGGI
            case 2:{
                //La lettura prima dello switch è il numero di messaggi
                num_mess = strtol(line, NULL, 10);
                temp_um->total = num_mess;
                
                read = getline(&line, &len, saved_messages);
                if(read == -1)
                    break;
                
                //Ho un for grande il triplo del numero di messaggi perchè per ogni messaggi ci sono 3 fasi
                for(int j = 0; j < num_mess * 3; j++){
                    //Evito la lettura nella fase 1, perchè già fatta nell'iterazione precedente
                    //o nel case SEND
                    if(j%3 != 0){
                        read = getline(&line, &len, saved_messages);
                        if(read == -1)
                            break;
                    }

                    //Fase 1, copia del messaggio
                    if(j%3 == 0){
                        strcpy(new_mess->mess, line);
                    }

                    //Fase 2, si legge se received, e ci si comporta di conseguenza
                    if(j%3 == 1){
                        new_mess->received = strtol(line, NULL, 10);
                        if(primo_non_ric == true && new_mess->received == false){
                            temp_um->to_read.pfVectorAdd(&temp_um->to_read, new_mess);
                            temp_um->total = 1;
                            primo_non_ric = false;
                        }
                        else{
                            temp_um->total++;
                        }
                    }

                    //Fase 3, si legge il timestamp
                    if(j%3 == 2){
                        new_mess->send_timestamp = strtoul(line, NULL, 10);
                        temp_um->message_list.pfVectorAdd(&temp_um->message_list, new_mess);
                        temp_um->last_timestamp = new_mess->send_timestamp;
                        
                        //Qui si fa la lettura per decidere il prossimo passo
                        //Se reiterarsi nel for o spostarsi nello switch più esterno
                        read = getline(&line, &len, saved_messages);
                        if(read == -1)
                            break;
                        if(strncmp(line, "mitt:", 5) == 0){
                            current_type = 1;
                            no_read = true;
                            break;
                        }
                        if(strncmp(line, "dest:", 5) == 0){
                            current_type = 0;
                            no_read = true;
                            break;
                        }
                        new_mess = (void*) malloc(sizeof(struct Message));
                    }
                }
            }
        }
    }
    fclose(saved_messages);
}

void restore(){
    restoreMessages();
    showChats();
    //restoreLogin();
}

void showChats(){
    struct StructMessage* temp_m;
    printf("Liste structMessage: ");
    for(int i = 0; i < messages.pfVectorTotal(&messages); i++){
        temp_m = (struct StructMessage*)messages.pfVectorGet(&messages, i);
        printf("sm: %s\n", temp_m->dest);
        showUserMessages(&temp_m->userMessagesList);
    }
    printf("\n");
}

void showUserMessages(vector *v){
    struct UserMessages* temp_um;
    printf("Lista mittenti: ");
    for(int i = 0; i < v->pfVectorTotal(v); i++){
        temp_um = (struct UserMessages*)v->pfVectorGet(v, i);
        printf("m: %s\n", temp_um->sender);
        showMessages(&temp_um->message_list);
    }
}

void showMessages(vector *v){
    struct Message* temp_m;
    printf("Lista messaggi: ");
    for(int i = 0; i < v->pfVectorTotal(v); i++){
        if(i != 0)
            printf(" -> ");
        temp_m = (struct Message*)v->pfVectorGet(v, i);
        printf("m: %s", temp_m->mess);
    }
}
