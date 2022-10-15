#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "./server/include/constants.h"
#include "./server/include/deviceCommands.h"
#include "./server/include/serverCommands.h"
#include "./server/include/record.h"
#include "./server/include/util.h"

#include "globals.h"

int listener, ret, addrlen, len, sd, current_s, port, command;
uint16_t lmsg, s_command;
    
fd_set master; 
fd_set read_fds; 
int fdmax; 

struct sockaddr_in my_addr, cl_addr;
char buffer[BUFFER_SIZE];

vector userRegister;

vector messages;
vector usersLink;

int main(int argc, char** argv){
    // porta di ascolto viene passata come argomento, se non passata si utilizza la 4242
    if(argv[1] != NULL)
        port = strtol(argv[1], NULL, 10);
    else
        port = 4242;

    vector_init(&userRegister);
    vector_init(&messages);
    vector_init(&usersLink);

    int ret, newfd, addrlen, len, k, choice; 
    char c_choice;

    restore();
    showServerMenu();

    /* Creazione socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        printf("Errore nella creazione del socket\n");
        exit(1);
    }

    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr)); 
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4242);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    if(ret < 0){
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
    FD_SET(0, &master); //STDIN

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
        for(current_s = 0; current_s <= fdmax; current_s++) { 

            // controllo se i è pronto 
            if(FD_ISSET(current_s, &read_fds)) { 

                // se i è il listener, ho ricevuto una richiesta di connessione
                // (un client ha invocato connect())
                if(current_s == sd) { 
                    
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
                if(current_s == 0){
                    c_choice = getc(stdin);
                    getc(stdin); //bug
                    choice = c_choice - '0';
                    //printf("Comando server: %d %c\n", choice, c_choice);
                    execServerCommand(choice);
                }
                // se non è il listener, 'i'' è un descrittore di socket 
                // connesso che ha fatto la richiesta di orario, e va servito
                // ***senza poi chiudere il socket*** perché l'orario
                // potrebbe essere chiesto nuovamente al server
                if(current_s != sd && current_s != 0){ // socket di comunicazione
                    // ricevo la lunghezza del messaggio
                    ret = recv(current_s, (void*)&s_command, sizeof(uint16_t), 0);
                    if(ret == 0){
                        printf("CHIUSURA client rilevata!\n");
                        //      disconnessione_client(i);
                        fflush(stdout);
                        // il client ha chiuso il socket, quindi 
                        // chiudo il socket connesso sul server
                        close(current_s);
                        // rimuovo il descrittore newfd da quelli da monitorare
                        FD_CLR(current_s, &master);
                        continue;
                    }
                    if(ret < 0){
                        perror("ERRORE! \n");
                        // si è verificato un errore
                        close(current_s);
                        // rimuovo il descrittore newfd da quelli da monitorare
                        FD_CLR(current_s, &master);
                        continue;
                    }
                    command = ntohs(s_command);
                    printf("comando client rilevata: %d\n", command);
                    execDeviceCommand(command);
                    //showRegister();
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