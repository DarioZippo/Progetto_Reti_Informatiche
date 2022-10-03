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

void insertLoggedUser(char* username){
    int len = strlen(username);
    char record[1500];
    time_t rawtime;

    // cerco utente nella lista degli utenti hanno fatto una connessione
    bool found = false;
    int i;
    
    struct Record* temp;
    for (i = 0; i < userRegister.pfVectorTotal(&userRegister); i++)
    {
        temp = (struct Record*)userRegister.pfVectorGet(&userRegister, i);
        if(strcmp(temp->username, username) == 0){
            found = true;
            break;
        }
    }
    
    // se non c'è creo nuovo elemento della lista
    if(found == false){
        userRegister.pfVectorAdd(&userRegister, malloc(sizeof(struct Record)));
        printf("%d\n", userRegister.pfVectorTotal(&userRegister));
        temp = (struct Record*)userRegister.pfVectorGet(&userRegister, userRegister.pfVectorTotal(&userRegister) - 1);
    }
    printf("Temp individuato\n");
    // aggiorno entry del server
    time(&rawtime);

    strcpy(temp->username, username);
    temp->porta = 4242;//porta;
    temp->login = rawtime; // login a timestamp corrente
    temp->logout = (time_t) NULL; // logout NULL perchè è online
    temp->socket = i; // socket viene salvato per la disconnessione improvvisa
    writeLoginOnFile(username, record, len, 4242/*porta*/, rawtime);
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
        printf("Retrieved line of length %zu:\n", read);
        line[read - 1] = '\0';
        printf("%s\n", line);

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
    printf("Dopo la lettura\n");
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
    ret = recv(i, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione LEN_US: \n");
        return;
    }
    len = ntohs(lmsg);
    printf("Lunghezza us: %d\n", len);
    // riceve username
    ret = recv(i, (void*)username, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione US: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(i);
        return;
    }
    username[len] = '\0';
    // riceve lunghezza password
    ret = recv(i, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione LEN_P: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(i);
        return;
    }
    len = ntohs(lmsg);
    printf("Lunghezza ps: %d\n", len);
    // riceve password
    ret = recv(i, (void*)password, len, 0);
    if(ret < 0){
        perror("Errore in fase di ricezione P: \n");
        return;
    }
    if(ret == 0){
        clientDisconnection(i);
        return;
    }
    password[len] = '\0';

    printf("credentials: %s %s\n", username, password);
}

// funzione che chiude il socket di un client che si è disconnesso improvvisamente
// imposta anche il timestamp di logout uguale al timestamp corrente
// la funzione viene chiamata quando una recv restituisce 0
void clientDisconnection(int sock){
    time_t rawtime;
    
    // cerco nella lista degli utenti il client con quel socket
    bool found = false;
    int i;
    
    struct Record* temp;
    for (i = 0; i < userRegister.pfVectorTotal(&userRegister); i++)
    {
        temp = (struct Record*)userRegister.pfVectorGet(&userRegister, i);
        if(temp->socket == sock){
            found = true;
            break;
        }
    }
    if(found == false)
        return;

    temp->logout = time(&rawtime); // imposto timestamp logout
    
    perror("Discussione client \n");
    close(sock); // chiudo socket
    FD_CLR(sock, &master);
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
        insertLoggedUser(current_username);
        i = 0;
    }
    // se non l'ha trovato, invia "NO" e il client rileva che il login ha fallito
    if(found == false){
        /*ret = send(i, "NO\0", 6, 0);
        if(ret < 0){
            printf("Errore nell'invio\n");
            return 0;
        }*/
    }
    fclose(file_user);
    printf("Fine ripristino:\n");
}

void restore(){
    restoreLogin();
}