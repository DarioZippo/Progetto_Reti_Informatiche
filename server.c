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

#include "./server/include/globals.h"

int main(int argc, char** argv){
    // porta di ascolto viene passata come argomento, se non passata si utilizza la 4242
    if(argc > 2)
        port = strtol(argv[2], NULL, 10);
    else
        port = 4242;

    printf("************ SERVER STARTED ************\n");

    userRegisterInit(&userRegister);
    vector_init(&messages);
    vector_init(&usersLink);

    int ret, sd, newsd, addrlen, choice;
    char c_choice;

    restore();
    showServerMenu();

    // Creazione socket di ascolto
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
    
    // azzeramento fd_set
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    
    // inserisco i socket che servono nel master
    // setto opportunamente fdmax
    FD_SET(sd, &master);
    FD_SET(STDIN, &master);

    fdmax = sd; 
    
    //main loop 
    while(1){
    
        // Inizializzo il set read_fds, manipolato dalla select()
        read_fds = master; 
        
        // dopo la select in read_fds rimangono solo i file descriptor dei socket pronti
        ret = select(fdmax+1, &read_fds, NULL, NULL, NULL);
        if(ret<0){
            perror("ERRORE SELECT:");
            exit(1);
        }
        for(current_s = 0; current_s <= fdmax; current_s++) {
            // controllo se current_s è pronto 
            if(FD_ISSET(current_s, &read_fds)) { 
                // se current_s == listener_sock significa che ci sono nuove connessioni, perciò faccio accept 
                if(current_s == sd) { 
                    
                    printf("Nuovo client rilevato!\n");
                    fflush(stdout);
                    addrlen = sizeof(cl_addr);
                    // faccio accept() e creo il socket connesso 'newsd'
                    newsd = accept(sd,
                    (struct sockaddr *)&cl_addr, &addrlen);
                    
                    // Aggiungo il descrittore al set dei socket monitorati
                    FD_SET(newsd, &master); 
                    
                    // Aggiorno l'ID del massimo descrittore
                    if(newsd > fdmax){ 
                        fdmax = newsd;
                    }
                } 
                // file descriptor stdin, è stato digitato un comando
                if(current_s == STDIN){
                    scanf("%d", &choice);
                    getchar();
                    execServerCommand(choice);
                }
                // socket di comunicazione
                if(current_s != sd && current_s != STDIN){
                    // ricevo la lunghezza del messaggio
                    ret = recv(current_s, (void*)&s_command, sizeof(uint16_t), 0);
                    if(ret == 0){
                        printf("CHIUSURA client rilevata!\n");
                        clientDisconnection(current_s);
                        continue;
                    }
                    if(ret < 0){
                        perror("ERRORE! \n");
                        // si è verificato un errore
                        clientDisconnection(current_s);
                        // rimuovo il descrittore newsd da quelli da monitorare
                        FD_CLR(current_s, &master);
                        continue;
                    }
                    command = ntohs(s_command);
                    printf("comando client rilevata: %d\n", command);
                    execDeviceCommand(command);
                } 
            }
        }
        
    }
    
    printf("FINE\n");
    close(sd);
 
}