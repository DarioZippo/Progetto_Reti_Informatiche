#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./device/device.h"
#include "./device/chat_state.h"

typedef int bool;
#define true 1
#define false 0

struct ChatState chatState;
chatState.chat_on = false;

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

void chat(){
    printf("Apertura chat con il primo messaggio:\n");
    char message[1024];
    fgets(message, 1024, stdin);
    int len_message = strlen(message);

    // invio il comando al server, poi il messaggio (prima mando la lunghezza del messaggio)
    // MANDO UN INTERO COME COMANDO
    uint16_t s_command = htons(3);
    int ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    // invio lunghezza del messaggio e poi il messaggio
    lmsg = htons(len_message);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    ret = send(sd, message, message, 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
}

void execDeviceCommand(int command){
    switch (command)
    {
    case 1:
        //hanging();
        break;
    case 2:
        //show();
        break;
    case 3:
        chat();
        break;
    default:
        break;
    }
}

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

    //ricevi_chi_ha_fatto_show();

    // creo socket di ascolto
    // ogni peer ha un socket di ascolto per ricevere messaggi, 
    // ad esempio accettare richieste connessione per chat P2P
    listener_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listener_sock == -1){
        printf("Errore nella creazione del socket\n");
        exit(1);
    }

    // struttura my_addr con info su questo host
    // inizializzo struttura my_addr e faccio bind
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    port_16_bit = port;
    my_addr.sin_port = htons(port_16_bit);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr);
    ret = bind(listener_sock, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    if(ret<0){
        perror("Errore bind: \n");
        exit(1);
    }
    ret = listen(listener_sock, 10); // metto socket di ascolto effettivamente in ascolto
    if(ret<0){
        printf("Errore listen\n");
        exit(1);
    }

    // azzeramento fd_set
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // inserisco i socket che servono nel master
    // setto opportunamente fdmax
    FD_SET(listener_sock, &master); 
    FD_SET(0, &master);
    //fdmax = listener_sock;
    FD_SET(sd, &master);
    fdmax = (listener_sock > sd)? listener_sock : sd;

    while(1){
        read_fds = master; // read_fd serve per non modificare master    
        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL); // in read_fds rimangono solo socket pronti in lettura
        if(ret<0){
            perror("ERRORE SELECT:");
            exit(1);
        }
        for(i = 0; i <= fdmax; i++){ 
            if(FD_ISSET(i, &read_fds)){
                // se i == sock_ascolto significa che ci sono nuove connessioni, perciò faccio accept 
                if(i == listener_sock) {
                    len = sizeof(peer_addr);
                    new_sd = accept(listener_sock, (struct sockaddr*) &peer_addr, (socklen_t*)&len);
                    FD_SET(new_sd, &master); 
                    if(new_sd > fdmax)
                        fdmax = new_sd;
                    /*
                    if(chat_gruppo_attiva == 1)
                        socket_componenti[num_componenti++] = new_sd;
                    */
                }
            }
            // se i == 0, stdin è pronto in lettura, l'utente ha digitato un comando oppure inviato un messaggio
            if(i == 0){
                // quando c'è una chat in corso non si possono digitare altri comandi
                // se c'è una chat di gruppo in corso i messaggi vengono inviati al gruppo
                if(chatState.chat_on == true){
                    /*
                    fgets(command, 1024, stdin);
                    chatP2P(chatState.last_chat_sock);
                    */
                }
                else{
                    int choice = read(0, (void*)command, 1);
                    execUserCommand((int) choice);
                }
            }
            // Negli altri casi un socket di comunicazione è pronto
            // sto ricevendo messaggi da una chat P2P
            // la connessione perciò è già stata effettuata precedentemente
            if(i != listener_sock && i != 0){
                // mittente e messaggio possono essere al più 1024 caratteri
                char sender[1024], message[1024];

                ret = recv(i, (void*)sender, 1024, 0); // ricevo il mittente del messaggio e lo salvo in mittente
                if(ret < 0){
                    perror("Errore in fase di ricezione: \n");
                    continue;
                }
                // se recv restituisce 0 significa che l'altro socket è stato chiuso
                if(ret == 0){
                    peerDisconnection(i); // gestisce disconnessioni improvvise di peer con cui sono connesso
                    continue;
                }
                // se invece di un mittente ho ricevuto FINE è il segnale che la chat deve essere chiusa
                if(strcmp(sender, "FINE") == 0){
                    printf("FINE CHAT\n");
                    close(i); // chiudo socket
                    FD_CLR(i, &master); // lo tolgo dall'insieme dei file descriptor da controllare
                    //chat_attiva = 0; // smetto di chattare
                    //chat_gruppo_attiva = 0; // smetto di chattare con gruppo
                    continue;
                }
                
                // se invece di un mittente ho ricevuto GRUPPO è il segnale viene creato un gruppo
                // oppure è stato aggiunto un nuovo componente al gruppo
                //if(strcmp(mittente, "GRUPPO") == 0){
                
                // se invece di un mittente ho ricevuto SHARE è il segnale che mi stanno inviando un file
                /*
                if(strcmp(mittente, "SHARE") == 0){
                    printf("INIZIO SHARE\n");
                    ricevi_file_share(); // funzione per ricevere file
                    continue;
                } 
                */

                // se non sono entrato negli if precedenti significa che ho ricevuto un messaggio
                // Controlli sul tipo di messaggi ricevuti da implementare

                // aggiorno variabili per gestione chat
                strcpy(chatState.last_chat_peer, sender);
                chatState.last_chat_sock = i;
                //chat_attiva = 1;
                // ricevo mittente
                ret = recv(i, (void*)message, 1024, 0);
                if(ret < 0){
                    perror("Errore in fase di ricezione: \n");
                    continue;
                }
                // Gestione disconnessione improvvisa peer
                if(ret == 0){
                    peerDisconnection(i);
                    continue;
                }

                printf("%s: %s", sender, message);
            }
        }
    }
}