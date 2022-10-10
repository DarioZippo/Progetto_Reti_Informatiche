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

void chatStateInit(){
    chatState.chat_on = false;
}

void sendCredentials(char* credentials, int command){
    char current_username[BUFFER_SIZE], password[BUFFER_SIZE], cmd[7];
    int current_username_len, password_len, i, divider = -1;
    int lmsg; // variabili per l'invio della lunghezza delle stringhe
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

    // copio l'username in una variabile globale, servirà nella OUT
    strcpy(username, current_username);
    username_len = current_username_len;
    username[username_len] = '\0';

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
    return;
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
    logged = true;
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

// funzione per gestire la disconnessione di un peer con cui sono connesso
// sock è il socker del peer per la comunicazione con il peer appena disconnesso
void peerDisconnection(int sock){
    printf("DISCONNESSIONE PEER\n");
    // se ero connesso stavo avendo una chat o una chat di gruppo con lui
    // dato che finisce la chat aggiorno variabili di chat e chat di gruppo
    chatState.chat_on = 0;
    //chat_gruppo_attiva = 0;
    close(sock); // chiudo il socket
    FD_CLR(sock, &master); // lo elimino dai socket che controllo con select
    if(sock == sd){
        printf("SERVER DISCONNESSO\n");
        exit(0);
    }  
}

bool searchContact(char* user){
    FILE* file_contacts;
    char *line, *current_contact;
    char path[150];
    int read;
    bool found = false;

    strcpy(path, "./device/contacts/");
    strcat(path, username);
    strcat(path, ".txt");

    printf("%s\n", path);
    
    file_contacts = fopen(path, "r");
    if(file_contacts == NULL){
        printf("Errore nell'apertura del file\n");
        return false;
    }

    while ((read = getline(&line, &len, file_contacts)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        line[read - 1] = '\0';
        printf("%s\n", line);

        if(strncmp(line, user, strlen(line)) == 0){ 
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

    fclose(file_contacts);
    printf("Fine ricerca: %d\n", found);
    return found;
}

void chat(){
    printf("Apertura chat con il primo messaggio\n"
        "Inserire lo username del destinatario:\n");
    char dest[1024];
    int dest_len, p, send_port;
    uint16_t pp;
    fgets(dest, 1024, stdin);

    dest_len = strlen(dest);

    bool found = searchContact(dest);
    if(found == false){
        printf("Il contatto non è in rubrica\n");
        return;
    }

    printf("Trovato! Ora manda il messaggio:\n");
    char message[1024];
    fgets(message, 1024, stdin);
    int len_message = strlen(message);

    // 1)invio il comando al server
    // 2)invio username destinatario
    // 3)invio messaggio
    // MANDO UN INTERO COME COMANDO
    uint16_t s_command = htons(5);
    int ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    lmsg = htons(dest_len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    ret = send(sd, dest, dest_len, 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    // Il server mi risponde con la porta su cui si è connesso il client con cui voglio chattare
    // la porta vale 0 nel caso in cui quel client sia offline
    ret = recv(sd, (void*)&pp, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore nella ricezione\n");
        return;
    }
    // Gestione disconnessione server
    if(ret == 0){
        printf("DISCONNESSIONE SERVER\n");
        exit(1);
    }

    // se il destinatario è ONLINE ricevo la sua porta, qualora valesse 0 significa che è OFFLINE 
    p = ntohs(pp);
    dest[dest_len - 1] = '\0';
    if(p == 0){
        printf("%s è OFFLINE\nPuoi inviare comunque e %s li ricevera' quando tornera' online\n", dest, dest);
        send_port = server_port; // se è offline mando i messaggi al server
        //invia_dati_a_server(mio_username, username);
    }
    else{
        printf("%s è ONLINE\nINIZIO CHAT con %s\n", dest, dest);
        send_port = p;
    }

    // sono dentro la chat, invio messaggi finchè l'utente non digita \q
    if(send_port == server_port){
        // caso in cui l'altro utente è offline e mando messaggi a server
        while(1){
            char path[1050];
            FILE* chat_file;

            strcpy(path, "./device/chat_");
            strcat(path, username);
            strcat(path, "/");
            strcat(path, dest);
            strcat(path, ".txt");
            chat_file = fopen(path, "a");

            if(strcmp(message, "\\q\n") == 0){
                printf("USCITO dalla chat\n");
                break;
            }

            fprintf(chat_file, "0\n%s", message);
            fclose(chat_file);

            sendMessageToServer(username, dest, message); // funzione per inviare messaggio a server
            
            fgets(message, 1024, stdin); // prendo messaggio da stdin
            //Il primo messaggio è stato inviato PRIMA del while!
        }
    }
    /*
    else{ // caso in cui l'altro utente è online e faccio chat P2P
        int nuovo_sd;
        struct sockaddr_in peer/*, mio_peer*/;
    /*    uint16_t p;

        // in peer salvo le info relativo al peer con cui mi voglio connettere
        p = send_port;
        memset(&peer, 0, sizeof(peer));
        peer.sin_family = AF_INET;
        peer.sin_port = htons(p);
        inet_pton(AF_INET, "127.0.0.1", &peer.sin_addr);

        // nuovo_sd --> creo nuovo socket per la comunicazione P2P
        nuovo_sd = socket(AF_INET, SOCK_STREAM, 0);
        if(nuovo_sd == -1){
            printf("ERRORE CREAZIONE SOCKET\n");
            exit(1);
        }
        p = port; 

        // mi connetto con l'altro peer
        ret = connect(nuovo_sd, (struct sockaddr*)&peer, sizeof(peer));
        if(ret < 0){
            perror("Errore nella connessione con peer\n");
            exit(1);
        }
        porta_ultimo_peer_chat = porta_invio; 
        printf("CONNESSO CON ALTRO PEER %d\n", porta_ultimo_peer_chat);

        // aggiorno variabili per gestione chat
        chat_attiva = 1;
        strcpy(ultimo_peer_chat, username); 
        sock_ultima_chat = nuovo_sd;
        FD_SET(nuovo_sd, &master);
        if(nuovo_sd > fdmax)
            fdmax = nuovo_sd;

        leggi_cronologia_messaggi(username);

        fgets(comando, 1024, stdin); // prendo messaggio da stdin
        // funzione che manda un messaggio all'altro peer
        // ha come argomento il socker per la comunicazione
        chatP2P(nuovo_sd); 
    }
    */
}

// funzione per mandare dati per preparare il server a ricevere messaggi pendenti
// si utilizza quando faccio chat username, ma username è offline
void sendMessageToServer(char* sender, char* dest, char* message){
    // Invio il comando PENDENTE
    uint16_t s_command = htons(10);
    int ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }

    /*
    // invio lunghezza messaggio e poi codice a server
    lmsg = htons(9);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }
    */

    // invio mittente
    ret = send(sd, (void*)sender, 1024, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }

    // invio destinatario
    ret = send(sd, (void*)dest, 1024, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }

    //invio messaggio
    ret = send(sd, (void*)message, 1024, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
         return;
    }
}

void execUserCommand(char command){
    printf("exec: %c\n", command);
    switch (command)
    {
    case '3':
        //hanging();
        break;
    case '4':
        //show();
        break;
    case '5':
        chat();
        break;
    default:
        break;
    }
}

int main(int argc, char** argv){
    uint16_t port_16_bit;
    int choice;
    char command;

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

    chatStateInit();

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
        perror("Errore bind 1: \n");
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
                // se i == listener_sock significa che ci sono nuove connessioni, perciò faccio accept 
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
                // se i == 0, stdin è pronto in lettura, l'utente ha digitato un comando oppure inviato un messaggio
                else if(i == 0){
                    // quando c'è una chat in corso non si possono digitare altri comandi
                    // se c'è una chat di gruppo in corso i messaggi vengono inviati al gruppo
                    if(chatState.chat_on == true){
                        /*
                        fgets(command, 1024, stdin);
                        chatP2P(chatState.last_chat_sock);
                        */
                    }
                    else{
                        command = getc(stdin);
                        getc(stdin); //bug
                        printf("choice: %c\n", command);
                        execUserCommand(command);
                    }
                }
                // Negli altri casi un socket di comunicazione è pronto
                // sto ricevendo messaggi da una chat P2P
                // la connessione perciò è già stata effettuata precedentemente
                else if(i != listener_sock && i != 0){
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
}