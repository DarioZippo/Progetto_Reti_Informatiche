#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "device.h"

#define BUFFER_SIZE 1024
#define RESPONSE_LEN 9 // HH:MM:SS\0

typedef int bool;
#define true 1
#define false 0

int sd, ret, port, len, new_sd;
uint16_t lmsg;
struct sockaddr_in server_addr, my_addr, peer_addr;
char* username;
int username_len;
bool logged;
fd_set master; 
fd_set read_fds;
int fdmax, i, listener_sock;

void readCredentials(char* credentials){
    printf("Inserire credenziali separate da uno spazio\n");
    fgets(credentials, BUFFER_SIZE, stdin);
    return credentials;
}

void sendCredentials(char* credentials, int command){
    char current_username[BUFFER_SIZE], password[BUFFER_SIZE], cmd[7];
    int current_username_len, password_len, i, divider = -1;
    int len, lmsg; // variabili per l'invio della lunghezza delle stringhe
    // cerco gli spazi che indicano la separazione tra comando-username-password
    for(i = 0; i < strlen(credentials); i++){
        if(credentials[i] == ' '){
            if(divider == -1)
                divider = i;
        }
    }

    // attraverso le posizioni degli spazi copio in delle variabili username e password
    current_username_len = divider - 1;
    password_len = strlen(credentials) - divider - 1;
    // copio l'username in una variabile globale, servirà nella OUT
    strncpy(username, current_username, username_len);
    //strncpy(password, current_password, password_len);
    username_len = current_username_len;
    current_username[current_username_len] = '\0';
    // invio il comando al server, poi username e passowrd (prima mando la lunghezza del messaggio)
    //len = strlen(comando); 
    switch (command)
    {
    case 1:
        len = 3;
        lmsg = htons(3);
        break;
    case 2:
        len = 7;
        lmsg = htons(7);
        break;
    default:
        break;
    }
    int ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    switch (command)
    {
    case 1:
        strcpy(cmd, "IN\0");
        break;
    case 2:
        strcpy(cmd, "SIGNUP\0");
        break;
    default:
        break;
    }
        
    ret = send(sd, cmd, len, 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    // invio lunghezza dell'username e poi l'username
    lmsg = htons(current_username_len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    ret = send(sd, current_username, current_username_len, 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    // invio lunghezza della passwrod e poi la password
    lmsg = htons(password_len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    ret = send(sd, password, password_len, 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
}

void signup(){
    printf("Form di registrazione...\n");
    char credentials[BUFFER_SIZE]; 
    readCredentials(credentials);
    sendCredentials(credentials, 1);
}

void in(){
    printf("Form di accesso...\n");
    char credentials[BUFFER_SIZE]; 
    readCredentials(credentials);
    sendCredentials(credentials, 2);

    //Gestione della connessione post
}

int accessMenu(){
    printf("Scegli fra le seguenti opzioni:\n" 
    "1: Registrati come nuovo utente\n" 
    "2: Accedi come utente già registrato\n");
    int choice;
    int min = 1, max = 2;
    do{
        scanf("%d", &choice);
        getchar(); //Risolve un bug tra scanf e fgets eseguiti in successione
    }while(choice < min || choice > max);
    return choice;
}

void deviceAccess(){
    int choice = accessMenu();
    switch (choice)
    {
    case 1:
        signup();
        break;
    case 2:
        in();
        break;
    default:
        printf("Errore nello switch, choice: %d", choice);
        break;
    }
}

/*
//Inizializzazione dello struct Server
void device_init(struct Device *d){
    //init function pointers
    d->pfSignup = signup;
    d->pfIn = in;
    //initialize the capacity and allocate the memory
    printf("Device inizzializzato\n");
}
*/
int main(int argc, char* argv[]){
    //device_init(&d);

    int ret, sd, len;
    bool logged;
    struct sockaddr_in srv_addr;
    char buffer[BUFFER_SIZE];

    /* Creazione socket */
    sd = socket(AF_INET,SOCK_STREAM,0);

    /* Creazione indirizzo del server */
    memset(&srv_addr, 0, sizeof(srv_addr)); 
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    /* connessione */
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0){
       perror("Errore in fase di connessione: \n");
       exit(1);
    }

    do{
        deviceAccess();
    }while(logged == false);

    //Continuo
}