#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "device.h"

void sendCredentials(char* credentials, int command){
    char current_username[BUFFER_SIZE], password[BUFFER_SIZE], cmd[7];
    int current_username_len, password_len, i, divider = -1;
    int len, lmsg; // variabili per l'invio della lunghezza delle stringhe
    // cerco gli spazi che indicano la separazione tra comando-username-password
    for(i = 0; i < strlen(credentials); i++){
        if(credentials[i] == ' '){
            divider = i;
        }
    }

    // attraverso le posizioni degli spazi copio in delle variabili username e password
    current_username_len = divider;
    //printf("Strlen = %d", strlen(credentials));
    password_len = (strlen(credentials) - divider) -2;
    // copio l'username in una variabile globale, servirà nella OUT
    strncpy(current_username, credentials, current_username_len);
    strncpy(password, &(credentials[divider+1]), password_len);
    username_len = current_username_len;
    current_username[current_username_len] = '\0';
    // invio il comando al server, poi username e passowrd (prima mando la lunghezza del messaggio)
    // MANDO UN INTERO COME COMANDO
    uint16_t s_command = htons(command);
    int ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    // COMMENTATA LA SOLUZIONE STRINGA
    /*
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
        perror("Errore in fase di LEN comando: \n");
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
        perror("Errore in fase di invio STRINGA comando: \n");
        exit(1);
    }
    */
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

void readCredentials(char* credentials){
    printf("Inserire credenziali separate da uno spazio\n");
    fgets(credentials, BUFFER_SIZE, stdin);
    return credentials;
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
int main(int argc, char** argv){ 
    uint16_t port_16_bit;

    // se il server è in ascolto su una porta diversa da 4242 deve essere passata come secondo argomento
    if(argc > 2)
        port = strtol(argv[2], NULL, 10); // conversione da stringa a intero
    else
        port = 4242;
    // la porta su cui si connette il client deve essere passata come primo argomento
    // deve essere passata obbligatoriamente altrimenti genererà errore
    if(argc > 1)
        port = strtol(argv[1], NULL, 10); // conversione da stringa a intero
    else{
        printf("Devi inserire il numero di porta\n");
        //return -1;
    }

    char buffer[BUFFER_SIZE];

    /* Creazione socket */
    sd = socket(AF_INET,SOCK_STREAM,0);
    if(sd == -1){
        printf("Errore nella creazione del socket\n");
        exit(1);
    }    

    /* Creazione indirizzo del server */
    memset(&server_addr, 0, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    /* connessione */
    ret = connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0){
       perror("Errore in fase di connessione: \n");
       exit(1);
    }

    do{
        deviceAccess();
    }while(logged == false);

    //Continuo
}