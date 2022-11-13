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
#include "./../include/message.h"

#include "./../include/globals.h"

void help(){
    printf("Comandi disponibili:\n"
        "1) help --> mostra i dettagli dei comandi\n"
        "2) list --> mostra un elenco degli utenti connessi\n"
        "3) esc --> chiude il server;\n"
    );
}

void showRegister(){
    printf("Utenti connessi: \n");
    struct Record* temp;
    bool first = true;
    for(int i = 0; i < userRegister.records.pfVectorTotal(&userRegister.records); i++){
        temp = (struct Record*)userRegister.records.pfVectorGet(&userRegister.records, i);
        if(temp->logout == (time_t) NULL){ // logout == NULL significa che è online
            if(first == false){
                printf(" -> ");
            }
            printf(" %s*%s*%d ", temp->username, strtok(ctime(&temp->login), "\n"), temp->port);
            first = false;
        }
    }
    printf("\n");
    
    //showUsersLinks();
}

// comando esc
// server si disconnette
// salva i messaggi pendenti in saved_messages.txt
void esc(){
    printf("ESC\n");

    FILE* saved_messages;
    time_t rawtime;
    bool first_dest_chat_message;

    struct Record* temp_r;
    for (int i = 0; i < userRegister.records.pfVectorTotal(&userRegister.records); i++){
        temp_r = userRegister.records.pfVectorGet(&userRegister.records, i);
        if(temp_r->logout == (time_t) NULL){
            temp_r->logout = time(&rawtime);
            close(temp_r->socket);
        }
    }

    struct StructMessage* temp_sm;
    vector *temp_uml;
    struct UserMessages* temp_um;
    vector *temp_ml;
    struct Message* temp_m;

    saved_messages = fopen("./server/documents/saved_messages.txt", "w");
    for(int i = 0; i < messages.pfVectorTotal(&messages); i++){
        //printf("For 1\n");
        first_dest_chat_message = true;
        temp_sm = (struct StructMessage*)messages.pfVectorGet(&messages, i);
        
        temp_uml = &temp_sm->userMessagesList;
        for(int j = 0; j < temp_uml->pfVectorTotal(temp_uml); j++){
            //printf("For 2\n");

            temp_um = (struct UserMessages*)temp_uml->pfVectorGet(temp_uml, j);
            temp_ml = &temp_um->message_list;
            for(int k = 0; k < temp_ml->pfVectorTotal(temp_ml); k++){
                //printf("For 3\n");                
                temp_m = (struct Message*)temp_ml->pfVectorGet(temp_ml, k);
                
                if(temp_m->received == false){
                    //printf("For 4\n"); 
                    if(first_dest_chat_message == true){
                        //printf("For 5\n"); 
                        fprintf(saved_messages, "dest:\n%s\n", temp_sm->dest);

                        first_dest_chat_message = false;
                    }
                    if(k == 0){
                        fprintf(saved_messages, "mitt:\n%s\n", temp_um->sender);
                        fprintf(saved_messages, "%d\n", temp_um->total);
                    }

                    fprintf(saved_messages, "%s", temp_m->mess);
                    fprintf(saved_messages, "%d\n", temp_m->received);
                    fprintf(saved_messages, "%ld\n", temp_m->send_timestamp);
                }
            }
        }
    }
    fclose(saved_messages);
    exit(0);
}

void showServerMenu(){
    printf("Digita un comando:\n"
        "\n"
        "1) help --> mostra i dettagli dei comandi\n"
        "2) list --> mostra un elenco degli utenti connessi\n"
        "3) esc --> chiude il server;\n"
    );
}

void execServerCommand(int choice){
    fflush(stdout);
    switch (choice)
    {
    case 10:
        pendentMessage();
        break;
    case 1:
        help();
        break;
    case 2:
        showRegister();
        break;
    case 3:
        esc();
        break;
    default:
        printf("Input sbagliato");
        break;
    }
}

int inputServerMenu(){
    int choice;
    int min = 1, max = 3;
    do{
        scanf("%d", &choice);
        //getchar(); //Risolve un bug tra scanf e fgets eseguiti in successione
    }while(choice < min || choice > max);
    printf("Input dato\n");
    return choice;
}