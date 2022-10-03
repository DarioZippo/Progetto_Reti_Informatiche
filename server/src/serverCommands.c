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

void help(){
    printf("I need somebody\n"
        "(Help) not just anybody\n"
        "(Help) you know I need someone, help\n");
}

void showRegister(){
    struct Record* temp;
    printf("Utenti connessi: ");
    for (int i = 0; i < userRegister.pfVectorTotal(&userRegister); i++){
        if(i != 0)
            printf("->");
        temp = (struct Record*)userRegister.pfVectorGet(&userRegister, i);
        printf(" %s ", temp->username);
    }
    printf("\n");
}

void esc(){
    printf("Da grande voglio fare l'USCIERE!\n");
}

void showMenu(){
    printf("***************************** SERVER STARTED *********************************\n"
        "Digita un comando:\n"
        "\n"
        "1) help --> mostra i dettagli dei comandi\n"
        "2) list --> mostra un elenco degli utenti connessi\n"
        "3) esc --> chiude il);\n"
    );
}

void execServerCommand(int choice){
    switch (choice)
    {
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

int inputMenu(){
    int choice;
    int min = 1, max = 3;
    do{
        scanf("%d", &choice);
        //getchar(); //Risolve un bug tra scanf e fgets eseguiti in successione
    }while(choice < min || choice > max);
    printf("Input dato\n");
    return choice;
}