#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "server.h"

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
    fprintf(file_user, "%s %s", username, password);
    
    fclose(file_user);
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
        client_disconnection(i);
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
        client_disconnection(i);
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
        client_disconnection(i);
        return;
    }
    password[len] = '\0';

    printf("credentials: %s %s\n", username, password);
}

// funzione che chiude il socket di un client che si è disconnesso improvvisamente
// imposta anche il timestamp di logout uguale al timestamp corrente
// la funzione viene chiamata quando una recv restituisce 0
void client_disconnection(int sock){
    time_t rawtime;
    
    // cerco nella lista degli utenti il client con quel socket
    bool found = false;
    int i;
    /*
    User* temp;
    for (i = 0; i < s.users.pfVectorTotal(&s.users); i++)
    {
        if(s.users.pfVectorGet(&s, i) == sock){
            found = true;
            temp = s.users.pfVectorGet(&s, i);
            break;
        }
    }
    if(found == false)
        return;

    temp->logout = time(&rawtime); // imposto timestamp logout
    */
    perror("Discussione client \n");
    close(sock); // chiudo socket
    //FD_CLR(sock, &master);
    //printf("CLIENT DISCONNESSO %s %s", temp->username, ctime(&temp->logout));
}

void execCommand(int command){
    switch (command)
    {
    case 1:
        signup();
        break;
    
    default:
        break;
    }
}

/*
//Inizializzazione dello struct Server
void server_init(struct Server *s){
    //init function pointers
    s->pfSignup = signup;
    //initialize the capacity and allocate the memory
    vector_init(&(s->users));
    printf("Server inizzializzato\n");
}
*/

int main(){
    int ret, newfd, addrlen, len, k;
    
    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);

    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr)); 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4242);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    
    if( ret < 0 ){
        perror("Bind non riuscita\n");
        exit(0);
    }
    
    ret = listen(sd, 10);
    if(ret < 0){
        printf("Errore in fase di listen: \n");
        exit(-1);
    }
    
    // Reset FDs
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    
    // Aggiungo il socket di ascolto (listener), creato dalla socket() 
    // all'insieme dei descrittori da monitorare (master)
    FD_SET(sd, &master); 

    // Aggiorno il massimo
    fdmax = sd; 
    
    //main loop 
    while(1){
    
        // Inizializzo il set read_fds, manipolato dalla select()
        read_fds = master; 
        
        // Mi blocco in attesa di descrottori pronti in lettura
        // imposto il timeout a infinito
        // Quando select() si sblocca, in &read_fds ci sono solo
        // i descrittori pronti in lettura!
        ret = select(fdmax+1, &read_fds, NULL, NULL, NULL);
        if(ret<0){
            perror("ERRORE SELECT:");
            exit(1);
        }
        
        // Spazzolo i descrittori 
        for(i = 0; i <= fdmax; i++) { 

            // controllo se i è pronto 
            if(FD_ISSET(i, &read_fds)) { 

                // se i è il listener, ho ricevuto una richiesta di connessione
                // (un client ha invocato connect())
                if(i == sd) { 
                    
                    printf("Nuovo client rilevato!\n");
                    fflush(stdout);
                    addrlen = sizeof(cl_addr);
                    // faccio accept() e creo il socket connesso 'newfd'
                    newfd = accept(sd,
                    (struct sockaddr *)&cl_addr, &addrlen);
                    
                    // Aggiungo il descrittore al set dei socket monitorati
                    FD_SET(newfd, &master); 
                    
                    // Aggiorno l'ID del massimo descrittore
                    if(newfd > fdmax){ 
                        fdmax = newfd; 
                    }
                } 
                // se non è il listener, 'i'' è un descrittore di socket 
                // connesso che ha fatto la richiesta di orario, e va servito
                // ***senza poi chiudere il socket*** perché l'orario
                // potrebbe essere chiesto nuovamente al server
                else {
                    // ricevo la lunghezza del messaggio
                    ret = recv(i, (void*)&s_command, sizeof(uint16_t), 0);

                    if(ret == 0){
                        printf("CHIUSURA client rilevata!\n");
                        //      disconnessione_client(i);
                        fflush(stdout);
                        // il client ha chiuso il socket, quindi 
                        // chiudo il socket connesso sul server
                        close(i);
                        // rimuovo il descrittore newfd da quelli da monitorare
                        FD_CLR(i, &master);
                        continue;
                    }
                    if(ret < 0){
                        perror("ERRORE! \n");
                        // si è verificato un errore
                        close(i);
                        // rimuovo il descrittore newfd da quelli da monitorare
                        FD_CLR(i, &master);
                        continue;
                    }
                    command = ntohs(s_command);
                    printf("comando client rilevata: %d\n", command);
                    execCommand(command);
                }
                
            }
        
        }
        //break;
    }
    // ci arrivo solo se monitoro stdin (descrittore 0)
    // -> rompo il while e passo a chiudere il listener
    
    printf("CHIUDO IL LISTENER!\n");
    fflush(stdout);
    close(listener);
 
}