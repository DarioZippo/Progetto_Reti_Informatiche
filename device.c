#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./server/include/constants.h"
#include "./device/include/device.h"
#include "./device/include/chat_state.h"
#include "./device/include/vector.h"
#include "./device/include/globals.h"

void chatStateInit(){
    chatState.chat_on = false;
    chatState.group_chat_on = false;
}

//Invio le credenziali al server, insieme ad un intero che identifica il comando
//1 per signup, 2 per login
void sendCredentials(char* credentials, int command){
    char current_username[BUFFER_SIZE], password[BUFFER_SIZE];
    int current_username_len, password_len, i, divider = -1;
    int lmsg; // variabili per l'invio della lunghezza delle stringhe
    // cerco gli spazi che indicano la separazione tra username-password
    for(i = 0; i < strlen(credentials); i++){
        if(credentials[i] == ' '){
            divider = i;
        }
    }

    // attraverso le posizioni degli spazi copio in delle variabili username e password
    current_username_len = divider;
    //printf("len = %d", strlen(credentials));
    password_len = (strlen(credentials) - divider) -2;

    strncpy(current_username, credentials, current_username_len);
    strncpy(password, &(credentials[divider+1]), password_len);
    
    current_username[current_username_len] = '\0';
    // invio il comando al server, poi username e password (prima mando la lunghezza del messaggio)
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

    // copio lo username in una variabile globale in caso di login
    if(command == 2){
        strcpy(username, current_username);
        username_len = current_username_len;
        username[username_len] = '\0';
        printf("mio user: %s\n", username);
    }

    // invio lunghezza della password e poi la password
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
    char buf[6];
    readCredentials(credentials);
    sendCredentials(credentials, 2);
    
    // invia la porta su cui è connesso
    ret = send(sd, &port, sizeof(int), 0);
    if(ret < 0){
        printf("Errore nell'invio\n");
        return;
    }

    printf("Faccio login\n");
    ret = recv(sd, buf, 6, 0);
    if(ret < 0){
        printf("Errore ricezione\n");
        exit(1);
    }
    // Gestione disconnessione improvvisa server
    if(ret == 0){
        printf("DISCONNESSIONE SERVER\n");
        exit(1);
    }
    if(strcmp(buf, "LOGIN\0") != 0){
        printf("Errore Login\n");
        logged = false;
        return;
    }
    logged = true;
    printf("Login avvenuto con successo\n");
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
    chatState.chat_on = false;
    chatState.group_chat_on = false;
    close(sock); // chiudo il socket
    FD_CLR(sock, &master); // lo elimino dai socket che controllo con select
    if(sock == sd){
        printf("SERVER DISCONNESSO\n");
        exit(0);
    }  
}

bool searchContact(char* user){
    FILE* file_contacts;
    char *line;
    char path[150];
    int read;
    bool found = false;

    strcpy(path, "./device/contacts/");
    strcat(path, username);
    strcat(path, ".txt");

    //printf("%s\n", path);
    
    file_contacts = fopen(path, "r");
    if(file_contacts == NULL){
        printf("Errore nell'apertura del file\n");
        return false;
    }

    while ((read = getline(&line, &len, file_contacts)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        line[read - 1] = '\0';
        //printf("%s\n", line);

        if(strncmp(line, user, strlen(line)) == 0){ 
            found = true;
            break;
        }
    }
    //printf("Dopo la lettura\n");

    fclose(file_contacts);
    printf("Fine ricerca: %d\n", found);
    return found;
}

void chat(){
    printf("Apertura chat con il primo messaggio\n"
        "Inserire lo username del destinatario:\n");
    char dest[BUFFER_SIZE], message[BUFFER_SIZE];
    int dest_len, p, send_port;
    uint16_t pp;
    fgets(dest, BUFFER_SIZE, stdin);

    dest_len = strlen(dest);

    bool found = searchContact(dest);
    if(found == false){
        printf("Il contatto non è in rubrica\n");
        return;
    }

    printf("Trovato!\n");

    // 1)invio il comando al server
    // 2)invio username destinatario
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
    }
    else{
        printf("%s è ONLINE\nINIZIO CHAT con %s\n", dest, dest);
        send_port = p;
    }

    // sono dentro la chat, invio messaggi finchè l'utente non digita \q
    if(send_port == server_port){
        // caso in cui l'altro utente è offline e mando messaggi a server
        char path[1050];
        FILE* chat_file;

        strcpy(path, "./device/chat_");
        strcat(path, username);
        strcat(path, "/");
        strcat(path, dest);
        strcat(path, ".txt");
        while(1){
            chat_file = fopen(path, "a");

            fgets(message, BUFFER_SIZE, stdin);
            if(strcmp(message, "\\q\n") == 0){
                printf("USCITO dalla chat\n");
                break;
            }
            //printf("%s\n", path);
            fprintf(chat_file, "0\n%s", message);
            fclose(chat_file);

            sendMessageToServer(username, dest, message); // funzione per inviare messaggio a server
        }
    }
    
    else{ // caso in cui l'altro utente è online e faccio chat P2P
        int new_sd;
        struct sockaddr_in peer/*, mio_peer*/;
        uint16_t p;

        // in peer salvo le info relativo al peer con cui mi voglio connettere
        p = send_port;
        memset(&peer, 0, sizeof(peer));
        peer.sin_family = AF_INET;
        peer.sin_port = htons(p);
        inet_pton(AF_INET, "127.0.0.1", &peer.sin_addr);

        // nuovo_sd --> creo nuovo socket per la comunicazione P2P
        new_sd = socket(AF_INET, SOCK_STREAM, 0);
        if(new_sd == -1){
            printf("ERRORE CREAZIONE SOCKET\n");
            exit(1);
        }
        p = port; 

        // mi connetto con l'altro peer
        ret = connect(new_sd, (struct sockaddr*)&peer, sizeof(peer));
        if(ret < 0){
            perror("Errore nella connessione con peer\n");
            exit(1);
        }
        chatState.last_port_chat_peer = send_port; 
        printf("CONNESSO CON ALTRO PEER %d\n", chatState.last_port_chat_peer);

        // aggiorno variabili per gestione chat
        chatState.chat_on = true;
        strcpy(chatState.last_chat_peer, dest); 
        chatState.last_chat_sock = new_sd;
        FD_SET(new_sd, &master);
        if(new_sd > fdmax)
            fdmax = new_sd;

        readSentMessages(dest);

        fgets(message, BUFFER_SIZE, stdin); // prendo messaggio da stdin
        // funzione che manda un messaggio all'altro peer
        // ha come argomento il socker per la comunicazione
        chatP2P(new_sd, message);
    }
}

// funzione per fare chat P2P
void chatP2P(int new_sd, char message[BUFFER_SIZE]){
    char path[1050];
    FILE* chat_file;

    strcpy(path, "./device/chat_");
    strcat(path, username);
    strcat(path, "/");
    strcat(path, chatState.last_chat_peer);
    strcat(path, ".txt");
    chat_file = fopen(path, "a");

    // esco dalla chat
    if(strcmp(message, "\\q\n") == 0){
        ret = send(new_sd, "FINE\0", BUFFER_SIZE, 0);
        if(ret < 0){
            printf("Errore in fase di invio\n");
            return;
        }
        close(new_sd);
        FD_CLR(new_sd, &master);
        chatState.chat_on = false;
        printf("USCITO dalla chat\n");
        return;
    }
    // ricevo e visualizzo utenti online
    if(strcmp(message, "\\u\n") == 0){
        showOnlineUsers();
        return;
    }
    // aggiungo componente al gruppo
    if(strncmp(message, "\\a ", 3) == 0){
        addGroupMember(new_sd, message);
        return;
    }
    // condivido file
    if(strncmp(message, "share ", 6) == 0){
        share(chatState.last_chat_sock, message);
        return;
    }

    // salvo messaggio nel file della rispettiva chat
    // salvo messaggio e poi 1 che indica che è stato ricevuto
    fprintf(chat_file, "1\n%s", message);
    fclose(chat_file);

    // invio mittente e poi messaggio
    ret = send(new_sd, (void*)username, BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }
    ret = send(new_sd, (void*)message, BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }
}

// LA SHARE PUO' ESSERE CHIAMATA SOLO DURANTE UNA CHAT/CHAT DI GRUPPO
// funzione per condividere file con altro peer
// parametro sock è il socket con cui comunicare
void share(int sock, char* message){
    char path[BUFFER_SIZE], buf[10240];
    int divider = -1;
    long int size, sended = 0;
    size_t res;
    uint16_t len;
    FILE* fp;

    printf("SHARE\n");

    // invio comando share al peer
    // len = BUFFER_SIZE perchè l'altro peer si aspetta un mittente (gli username sono max BUFFER_SIZE caratteri)
    ret = send(sock, "SHARE\0", BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore invio\n");
        exit(1);
    }

    for(int i = 0; i < strlen(message); i++){
        if(message[i] == ' '){
            divider = i;
            break;
        }
    }

    // estraggo il path dal comando digitato
    memcpy((void*)path, (void*) message + divider + 1, strlen(message) - 1);
    path[strlen(path)-1] = '\0';
    printf("%s\n", path);

    // legge il file binario
    if ( (fp = fopen(path, "rb")) == NULL){
        printf("Errore apertura file\n");
        exit(1);
    }

    // calcolo della dimensione del file da condividere
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    // ripristino del puntatore all'inizio del file
    fseek(fp, 0, SEEK_SET);
    
    // manda il file in partizioni di 10240 bit
    // salva anche res qualore i bit letti fosseri minori di 10240
    while( (res = fread(buf, 1, 10240, fp)) > 0){
        sended+=res;
        printf("Sended: %ld\n", sended);
        // invia il numero di bit che ha letto
        len = htons(res);
        ret = send(sock, &len, sizeof(uint16_t), 0);
        if(ret < 0){
            printf("Errore invio\n");
            exit(1);
        }
        // invia i bit
        ret = send(sock, buf, res, 0);
        if(ret < 0){
            printf("Errore invio\n");
            exit(1);
        }
        buf[0] = '\0';
    }
    if (sended != size)
        perror("il file non è stato inviato correttamente: ");
    
    // Per segnalare che ho finito di inviare mando 0 come lunghezza del prossimo messaggio
    len = htons(0);
    ret = send(sock, &len, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore invio\n");
        exit(1);
    }

    fclose(fp);
}

// funzione per ricevere il file mandato con la share
void receiveSharedFile(int current_s){
    char buf[10240], path[1050];    
    uint16_t len;
    int size;
    FILE* fp;

    printf("INIZIO SHARE\n");

    // il file ricevuto avrà sempre lo stesso nome
    // sarà cura dell'utente rinominare il file opportunamente
    // una seconda share consecutiva causerebbe la sovrascrittura della prima
    strcpy(path, "./received_file_");
    strcat(path, username);
    if ((fp = fopen(path, "wb")) == NULL){
       printf("Errore apertura file");
       exit(1);
   }

    while(1){
        ret = recv(current_s, &len, sizeof(uint16_t), 0);
        if(ret < 0){
            printf("Errore ricezione\n");
            exit(1);
        }  
        if(ret == 0){
            peerDisconnection(current_s);
            return;
        }
        size = ntohs(len);
        printf("chunk size: %d\n", size);
        if(size == 0){
            break;
        }
        ret = recv(current_s, buf, size, 0);
        if(ret < 0){
            printf("Errore ricezione\n");
            exit(1);
        }
        if(ret == 0){
            peerDisconnection(current_s);
            return;
        }
        fwrite(buf, size, 1, fp); // scrivo il file binario
    }  

    fclose(fp);   
    printf("FINE SHARE\n");   
}

// funzione per mandare messaggio a tutti i componenti del gruppo
void groupChat(int new_sd, char* message){
    int index;
    
    // se una persona esce dal gruppo allora tutti i componenti escono dal gruppo
    // l'utente esce inoltre da tutte le chat
    if(strcmp(message, "\\q\n") == 0){
        for(index = 0; index < chatState.members_number; index++){
            ret = send(chatState.socket_group[index], "FINE", BUFFER_SIZE, 0);
            if(ret < 0){
                printf("Errore in fase di invio\n");
                return;
            }
            close(chatState.socket_group[index]);
            FD_CLR(chatState.socket_group[index], &master);
            chatState.chat_on = false;
            chatState.group_chat_on = false;
        }
        printf("USCITO dal gruppo e da tutte le chat\n");
        return;
    }
    if(strcmp(message, "\\u\n") == 0){
        showOnlineUsers();
        return;
    }
    if(strncmp(message, "\\a ", 3) == 0){
        addGroupMember(0, message); // parametro del socket non serve, serve solamente in fase di creazione del gruppo
        return;
    }
    
    if(strncmp(message, "share ", 6) == 0){
        // Faccio share con tutti i componenti
        for(index = 0; index < chatState.members_number; index++){
            share(chatState.socket_group[index], message); 
        }
        return;
    }
    
    // invio mittente e poi messaggio a tutti componenti
    for(index = 0; index < chatState.members_number; index++){
        ret = send(chatState.socket_group[index], (void*)username, BUFFER_SIZE, 0);
        if(ret < 0){
            printf("Errore in fase di invio\n");
            return;
        }
        ret = send(chatState.socket_group[index], (void*)message, BUFFER_SIZE, 0);
        if(ret < 0){
            printf("Errore in fase di invio\n");
            return;
        }
    }
}

void addGroupMember(int new_sd, char* message){
    char dest[BUFFER_SIZE];
    int dest_len, divider = -1, send_port, p, new_member;
    uint16_t pp;
    struct sockaddr_in peer;

    printf("AGGIUNTA AL GRUPPO\n");
    // cerco lo spazio che fa da separatore tra comando e username destinatario
    for(int i = 0; i < strlen(message); i++){
        if(message[i] == ' '){
            divider = i;
            break;
        }
    }

    // copio username destinatario nella variabile username
    dest_len = strlen(message) - divider - 1;
    strncpy(dest, &(message[divider+1]), dest_len);
    dest[dest_len] ='\0';
    //printf("Voglio aggiungere %s\ncon divider: %d\ndest_len: %d\n", dest, divider, dest_len);
    //Verifico che dest sia in rubrica
    bool found = searchContact(dest);
    if(found == false){
        printf("Il contatto non è in rubrica\n");
        return;
    }
    // invio comando e poi username destinatario

    // invio comando chat perchè il server non fa altro che dirmi se l'altro
    // peer è online oppure no ed eventualmente darmi la porta dove è connesso

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

    // ricevo la porta su cui è connesso il nuovo componente
    // la porta vale 0 in caso in cui il componente sia offline
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

    // se il destinatario è ONLINE ricevo la sua porta, qualora valesse -1 significa che è OFFLINE 
    p = ntohs(pp);
    dest[strlen(dest)-1] = '\0';
    if(p == 0){
        printf("%s è OFFLINE\nIMPOSSIBILE AGGIUNGERE %s al GRUPPO\n", dest, dest);
        send_port = server_port;
        return;
    }
    else{
        printf("%s è ONLINE\nAGGIUNGO %s al GRUPPO\n", dest, dest);
        send_port = p;
    }

    // in peer salvo le info relative al nuovo membro
    memset(&peer, 0, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(p);
    inet_pton(AF_INET, "127.0.0.1", &peer.sin_addr);

    // new_member --> socket per la comunicazione con il nuovo componente
    new_member = socket(AF_INET, SOCK_STREAM, 0);
    if(new_member == -1){
        printf("ERRORE CREAZIONE SOCKET\n");
        exit(1);
    }

    // connessione con il nuovo componente
    ret = connect(new_member, (struct sockaddr*)&peer, sizeof(peer));
    if(ret < 0){
        perror("Errore nella connessione con peer\n");
        exit(1);
    }

    // Invio prima di tutto il codice al nuovo componente perchè deve fare delle operazioni prima degli altri
    ret = send(new_member, "GRUPPO\0", BUFFER_SIZE, 0);
    if(ret < 0){
        perror("Errore nell'invio\n");
        exit(1);
    }    

    // al nuovo componente invio 0 per distinguerlo dagli altri, deve gestire diversamente rispetto agli altri
    pp = htons(0);
    ret = send(new_member, (void*)&pp, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore nell'invio\n");
        exit(1);
    }

    // sleep aggiunta per dare tempo al nuovo componente di aggiornare le variabili e mettersi in ascolto nuovamente
    // senza sleep gli altri componenti potrebbero fare connessioni prima che le variabili siano aggiornate
    sleep(2);

    if(chatState.group_chat_on == false){ // gruppo nuovo, inizializzo variabili per gestione gruppo
        chatState.socket_group[0] = new_sd;
        chatState.members_number = 1;
    }

    // in socket componenti ho tutti i socket per la comunicazione con i componenti del gruppo
    chatState.socket_group[chatState.members_number] = new_member;
    chatState.members_number++;
    chatState.group_chat_on = true;
    
    FD_SET(new_member, &master);
    if(new_member > fdmax)
        fdmax = new_member;

    // invio codice GRUPPO a tutti i componenti del gruppo, eccetto quello appena aggiunto
    for(int i = 0; i < chatState.members_number - 1; i++){
        ret = send(chatState.socket_group[i], "GRUPPO\0", BUFFER_SIZE, 0);
        if(ret < 0){
            perror("Errore nell'invio\n");
            exit(1);
        }
    }

    // invio la porta del nuovo componente a tutti i componenti del gruppo (eccetto quello appena aggiunto)
    // in modo che i componenti possano fare la connessione col componente appena aggiunto
    pp = htons(send_port);
    for(int i = 0; i < chatState.members_number - 1; i++){
        ret = send(chatState.socket_group[i], (void*)&pp, sizeof(uint16_t), 0);
        if(ret < 0){
            perror("Errore nell'invio\n");
            exit(1);
        }
    }
}

void groupUpdate(int current_s){
    uint16_t p_comp;
    int member_port, new_connection;
    struct sockaddr_in new_member;

    // riceve la porta del nuovo componente
    // in caso di creazione del gruppo, i 2 componenti non creatori ricevono la porta dell'altro peer
    ret = recv(current_s, (void*)&p_comp, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di ricezione: \n");
        exit(1);
    }
    // Gestione disconnessione improvvisa peer
    if(ret == 0){
        peerDisconnection(current_s);
        return;
    }
    member_port = ntohs(p_comp);
    // il peer che viene aggiunto riceve 0 per poterlo distinguere da i peer già nel gruppo
    // deve essere gestito diversamente
    if(member_port == 0){
        printf("SONO STATO AGGIUNTO A UN GRUPPO\nINIZIO CHAT CON GRUPPO\n");
        // socket i mi ha aggiunto al gruppo, lo salvo tra i socket dei componenti del gruppo
        chatState.socket_group[0] = current_s; 
        // inizializzo variabili per gestione gruppo
        chatState.members_number = 1; 
        chatState.group_chat_on = true;
        return; // questo peer non deve eseguire il codice sottostante
    }

    // codice che viene eseguito dai peer che non sono stati aggiunti con \a username

    // se chat_gruppo_attiva == 0, viene creato un nuovo gruppo, quindi inizializzo le variabili
    if(chatState.group_chat_on == false){
        printf("NUOVO GRUPPO CREATO\nINIZIO CHAT CON GRUPPO\n");
        chatState.group_chat_on = true;
        chatState.socket_group[0] = current_s;
        chatState.members_number = 1;
    }
    // un componente è stato aggiunto al gruppo
    // devo perciò stabilire una connessione con il peer appena aggiunto
    // conosco la porta del peer che è stato aggiunto, mi è stata appena inviata
    printf("NUOVO COMPONENTE AGGIUNTO AL GRUPPO\n");
    new_connection = socket(AF_INET, SOCK_STREAM, 0); // socket comunicazione con nuovo componente
    if(new_connection == -1){
        printf("Errore creazione socket\n");
        exit(1);
    }

    // new_member contiene info sul nuovo componente del gruppo 
    memset(&new_member, 0, sizeof(new_member));
    new_member.sin_family = AF_INET;
    new_member.sin_port = htons(member_port);
    inet_pton(AF_INET, "127.0.0.1", &new_member.sin_addr);

    // Faccio connessione con nuovo componente
    ret = connect(new_connection, (struct sockaddr*)&new_member, sizeof(new_member));
    if(ret < 0){
        perror("Errore nella connessione con il nuovo componente\n");
        exit(1);
    }
    printf("CONNESSO CON NUOVO COMPONENTE\n");
    // inserisco il socket tra quelli dei componenti del gruppo
    chatState.socket_group[chatState.members_number++] = new_connection;
    FD_SET(new_connection, &master); // aggiunto socket a master
    if(new_connection > fdmax) // aggiorno fdmax
        fdmax = new_connection;
}

void readSentMessages(char* dest){
    FILE* chat_file;
    char *line;
    char path[150];
    int read;
    bool receive_check = true;

    printf("CRONOLOGIA MESSAGGI\n\n");
    // file contenente cronologia dei messaggi con dest
    strcpy(path, "./device/chat_");
    strcat(path, username);
    strcat(path, "/");
    strcat(path, dest);
    strcat(path, ".txt");
    // apro in lettura, se il file non esiste significa che non è stato ancora creato
    // perciò non ci sono messaggi precedenti da visualizzare
    chat_file = fopen(path, "r");
    if(chat_file == NULL){
        printf("Nessun messaggio\n\n");
        return;
    }

    while ((read = getline(&line, &len, chat_file)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        line[read - 1] = '\0';
        //printf("%s\n", line);

        if(receive_check == false)
            printf("%s\n", line);
        
        if(receive_check == true){
            // 1 --> messaggio ricevuto, stampo **
            // 0 --> messaggio non ricevuto, stampo *
            if(strcmp(line, "1") == 0)
                printf("** ");
            if(strcmp(line, "0") == 0)
                printf("* ");
        }
        receive_check = (receive_check + 1) % 2;
    }

    fclose(chat_file);
    printf("\n");
}

// funzione per aggiornare il file della cronologia dei messaggi
// a seguito di una notifica di show
void updateSentMessages(char* dest){ 
    FILE* chat_file;
    char *line = NULL, read_str[1024];
    char path[1050];
    int read;
    bool not_received = false;
    int first_line = 0, lines_number = 0, j;

    // file contenente cronologia dei messaggi con dest
    strcpy(path, "./device/chat_");
    strcat(path, username);
    strcat(path, "/");
    strcat(path, dest);
    strcat(path, ".txt");
    
    chat_file = fopen(path, "r");
    if(chat_file == NULL){
        printf("Nessun messaggio\n\n");
        return;
    }
    chat_file = fopen(path, "r");
    if(chat_file == NULL){
        printf("Nessun messaggio\n\n");
        return;
    }

    // non posso scrivere subito il file dato, quando trovo il giusto punto per scrivere è già troppo tardi
    // dato che l'accesso ai file in UNIX è sequenziale, quando ho letto la riga che avrei dovuto modificare
    // se provassi a scrivere scriverei nella riga successiva
    while(1){
        line = fgets(read_str, 1024, chat_file);
        if(line == NULL)
            break;
        lines_number++; // conto il numero di righe, lo utlizzo successivamente
        //printf("%s\n", line);
        // il punto da cui cominicare a scrivere è quando trovo un messaggio non ricevuto (codice 0)
        if(strcmp(line, "0\n") == 0 && not_received == false){
            not_received = true; // non_ricevuto serve per evitare di entrare in questo if le volte successive
            first_line = lines_number - 1; // first_line --> indice della prima riga da modificare
        }
    }

    fclose(chat_file);

    // tutti i messaggi ricevuti, non c'è niente da modificare
    if(not_received == false){
        printf("Niente da aggiornare\n");
        return;
    }

    // apro il file sia in lettura che in scrittura, adesso conosco il numero di righe del file
    // e l'indice della prima riga da modificare
    // devo modificare solamente le righe contenenti i codici che indicano la ricezione o meno del messaggio
    //printf("first: %d, number: %d\n", first_line, lines_number);
    chat_file = fopen(path, "r+");
    for(j = 0; j < lines_number; j++){
        if(j < first_line) // ancora non devo modificare, perciò leggo solamente
            getline(&line, &len, chat_file);
        if(j >= first_line && (j-first_line)%2 == 0) // riga contenente il codice, scrivo 1
            fprintf(chat_file, "1\n");
        if(j >= first_line && (j-first_line)%2 != 0) // riga contenente un messaggio, leggo solamente
            getline(&line, &len, chat_file);
    }

    fclose(chat_file); // chiudo il file
}

//Funzione per ricevere la lista degli utenti che hanno fatto show
//e richiamare l'update del file di cronologia messaggi relativo
void receiveChatInfo(){
    char dest[BUFFER_SIZE];

    while(1){
        // il server invia gli username degli utenti che hanno eseguito la show
        ret = recv(sd, (void*)dest, BUFFER_SIZE, 0); // salvo l'username nella variabile dest
        if(ret < 0){
            perror("Errore in fase di ricezione: \n");
            exit(1);
        }
        // se recv restituisce 0 che il server si è disconnesso
        if(ret == 0){
            printf("SERVER DISCONNESSO\n");
            exit(1);
        }
        // quando ricevo \0 significa che la lista degli utenti che hanno eseguito la show è terminata
        if(strcmp(dest, "\0") == 0)
            break;
        printf("%s\n", dest);
        updateSentMessages(dest); // aggiorno il file della cronologia dei messaggi
    }
}

void showOnlineUsers(){
    char current_online_user[BUFFER_SIZE];

    printf("UTENTI ONLINE\n");

    // invio il comando per ricevere gli utenti online
    uint16_t s_command = htons(11);
    ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    //ricevo il numero di utenti online e stampo
    uint16_t s_online_counter;
    ret = recv(sd, (void*)&s_online_counter, sizeof(uint16_t), 0);
    int online_counter = ntohs(s_online_counter);

    printf("Numero utenti online: %d\n", online_counter);
    // ricevo da server tutti gli username
    // sd --> socket comunicazione con server
    for (int i = 0; i < online_counter; i++)
    {
        //printf("%d\n", i);
        ret = recv(sd, current_online_user, BUFFER_SIZE, 0);
        if(ret < 0){
            printf("Errore ricezione\n");
            exit(1);
        }
        if(ret == 0){
            printf("DISCONNESSIONE SERVER\n");
            exit(1);
        }
        printf("%s\n", current_online_user);
    }
}

void hanging(){
    char messages[1055]; // 1055: dim. massima stringa che arriva
    bool first = true; // variabile booleana per distinguere quando fare una stampa

    // invio il comando per l'hanging
    uint16_t s_command = htons(3);
    ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    // invio il mio username
    ret = send(sd, (void*)username, BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }

    printf("FORMATO: Username Numero Messaggi inviati Timestamp piu' recente\n");
    while(1){
        ret = recv(sd, (void*)messages, 1055, 0);
        if(ret < 0){
            perror("Errore in fase di ricezione: \n");
            exit(1);
        }
        if(ret == 0){
            printf("DISCONNESSIONE SERVER\n");
            exit(1);
        }
        // quando ricevo ZERO significa che non ci sono più messaggi
        if(strncmp(messages, "ZERO", 4) == 0){
            // variabile first serve per gestire la stampa sul terminale
            if(first == true)
                printf("Non hai nessun messaggio da visualizzare\n");
            break;
        }
        printf("%s\n", messages);
        strcpy(messages, "");
        first = false;
    }
        
}

void show(){
    char dest[BUFFER_SIZE], message[BUFFER_SIZE];
    int dest_len, i, mess_number;
    uint16_t s_number_mess;

    printf("SHOW\n"
        "Inserire lo username del destinatario:\n");
    fgets(dest, BUFFER_SIZE, stdin);

    // invio il comando per SHOW
    uint16_t s_command = htons(4);
    ret = send(sd, (void*) &s_command, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }

    // invio lunghezza username
    dest_len = strlen(dest) - 1;
    dest[dest_len] = '\0';
    
    lmsg = htons(dest_len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    ret = send(sd, dest, dest_len, 0);
    if(ret < 0){
        printf("Errore nella ricezione\n");
        return;
    }
    
    // invio lunghezza del mio username e poi il mio username
    printf("%s: %d, %d\n", username, username_len, strlen(username));
    lmsg = htons(username_len);
    ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    ret = send(sd, username, username_len, 0);
    if(ret < 0){
        perror("Errore in fase di invio comando: \n");
        exit(1);
    }
    
    // ricevo il numero di messaggi che sto per ricevere
    ret = recv(sd, (void*) &s_number_mess, sizeof(uint16_t), 0);
    if(ret < 0){
        printf("Errore nella ricezione\n");
        return;
    }
    if(ret == 0){
        printf("DISCONNESSIONE SERVER\n");
        exit(1);
    }
    mess_number = ntohs(s_number_mess);
    if(mess_number == 0){
        printf("Nessun messaggio\n");
        return;
    }

    printf("Messaggi che ti ha inviato mentri eri offline: %d\n", mess_number);
    // ricevo i messaggi, conoscendo il numero di messaggi non serve un codice per indicare la fine
    for(i = 0; i < mess_number; i++){
        ret = recv(sd, (void*)message, BUFFER_SIZE, 0);
        if(ret < 0){
            perror("Errore in fase di ricezione: \n");
            return;
        }
        if(ret == 0){
            printf("DISCONNESSIONE SERVER\n");
            exit(1);
        }
        printf("%s", message);
        strcpy(message, "");
    }
    printf("\n");
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
    ret = send(sd, (void*)sender, BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }

    // invio destinatario
    ret = send(sd, (void*)dest, BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
        return;
    }

    //invio messaggio
    ret = send(sd, (void*)message, BUFFER_SIZE, 0);
    if(ret < 0){
        printf("Errore in fase di invio\n");
         return;
    }
}

void out(){
    printf("OUT\n");
    exit(0);
}

void execUserCommand(char command){
    printf("exec: %c\n", command);
    switch (command)
    {
    case '3':
        hanging();
        break;
    case '4':
        show();
        break;
    case '5':
        chat();
        break;
    case '6':
        out();
    default:
        break;
    }
}

int main(int argc, char** argv){
    uint16_t port_16_bit;
    int current_s;
    char command, message[BUFFER_SIZE];

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
        return -1;
    }
    
    logged = false;

    chatStateInit();

    // sd --> socket con cui si connette con il server
    sd = socket(AF_INET,SOCK_STREAM,0);
    if(sd == -1){
        printf("Errore nella creazione del socket\n");
        exit(1);
    }    

    // inizializzazione server_addr contenente le info del server
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

    //Fase di registrazione ed autenticazione del client
    do{
        deviceAccess();
    }while(logged == false);

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

    ret = bind(listener_sock, (struct sockaddr*)&my_addr, sizeof(my_addr) );
    if(ret<0){
        perror("Errore bind 1: \n");
        exit(1);
    }
    // metto socket di ascolto effettivamente in ascolto
    ret = listen(listener_sock, 10);
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
    
    FD_SET(sd, &master);
    fdmax = (listener_sock > sd)? listener_sock : sd;

    while(1){
         // Inizializzo il set read_fds, manipolato dalla select()
        read_fds = master;    

        // dopo la select in read_fds rimangono solo i file descriptor dei socket pronti
        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        if(ret<0){
            perror("ERRORE SELECT:");
            exit(1);
        }
        for(current_s = 0; current_s <= fdmax; current_s++){ 
            if(FD_ISSET(current_s, &read_fds)){
                // se current_s == listener_sock significa che ci sono nuove connessioni, perciò faccio accept 
                if(current_s == listener_sock) {
                    len = sizeof(peer_addr);
                    new_sd = accept(listener_sock, (struct sockaddr*) &peer_addr, (socklen_t*)&len);
                    FD_SET(new_sd, &master); 
                    if(new_sd > fdmax)
                        fdmax = new_sd;
                    if(chatState.group_chat_on == true)
                        chatState.socket_group[chatState.members_number++] = new_sd;
                }
                // file descriptor stdin, è stato digitato un comando o inviato un messaggio
                else if(current_s == STDIN){
                    fgets(message, BUFFER_SIZE, stdin);
                    // quando c'è una chat in corso non si possono digitare altri comandi
                    // se c'è una chat di gruppo in corso i messaggi vengono inviati al gruppo
                    if(chatState.group_chat_on == true){
                        groupChat(chatState.last_chat_sock, message);
                    }
                    else if(chatState.chat_on == true){
                        chatP2P(chatState.last_chat_sock, message);
                    }
                    else{
                        command = message[0];
                        fflush(stdin);
                        printf("choice: %c\n", command);
                        execUserCommand(command);
                    }
                }
                // Negli altri casi un socket di comunicazione è pronto
                // sto ricevendo messaggi da una chat P2P
                // la connessione perciò è già stata effettuata precedentemente
                else if(current_s != listener_sock && current_s != STDIN){
                    char sender[BUFFER_SIZE];

                    ret = recv(current_s, (void*)sender, BUFFER_SIZE, 0); // ricevo il mittente del messaggio e lo salvo in mittente
                    if(ret < 0){
                        perror("Errore in fase di ricezione: \n");
                        continue;
                    }
                    // se recv restituisce 0 significa che l'altro socket è stato chiuso
                    if(ret == 0){
                        peerDisconnection(current_s); // gestisce disconnessioni improvvise di peer con cui sono connesso
                        continue;
                    }
                    // se invece di un mittente ho ricevuto NOTIFICA è il segnale per l'aggiornamento delle chat
                    if(strcmp(sender, "NOTIFICA") == 0){
                        printf("NOTIFICA\n");
                        receiveChatInfo();
                        continue;
                    }

                    // se invece di un mittente ho ricevuto FINE è il segnale che la chat deve essere chiusa
                    if(strcmp(sender, "FINE") == 0){
                        printf("FINE CHAT\n");
                        close(current_s); // chiudo socket
                        FD_CLR(current_s, &master); // lo tolgo dall'insieme dei file descriptor da controllare
                        chatState.chat_on = false; // smetto di chattare
                        chatState.group_chat_on = false; // smetto di chattare con gruppo
                        continue;
                    }
                    
                    // se invece di un mittente ho ricevuto GRUPPO è il segnale viene creato un gruppo
                    // oppure è stato aggiunto un nuovo componente al gruppo
                    if(strcmp(sender, "GRUPPO") == 0){
                        groupUpdate(current_s);
                        continue;
                    }
                    // se invece di un mittente ho ricevuto SHARE è il segnale che mi stanno inviando un file
                    if(strcmp(sender, "SHARE") == 0){
                        receiveSharedFile(current_s); // funzione per ricevere file
                        continue;
                    } 
                    // se non sono entrato negli if precedenti significa che ho ricevuto un messaggio
                    if(chatState.group_chat_on == true)
                        printf("Messaggio di gruppo ricevuto\n");
                    if(chatState.group_chat_on == false && (chatState.chat_on == 0 || strcmp(chatState.last_chat_peer, sender) != 0)){
                        printf("Messaggio di %s ricevuto\nINIZIO CHAT con %s\n", sender, sender);
                    }

                    // aggiorno variabili per gestione chat
                    strncpy(chatState.last_chat_peer, sender, strlen(sender));
                    chatState.last_chat_sock = current_s;
                    chatState.chat_on = true;

                    // ricevo mittente
                    ret = recv(current_s, (void*)message, BUFFER_SIZE, 0);
                    if(ret < 0){
                        perror("Errore in fase di ricezione: \n");
                        continue;
                    }
                    // Gestione disconnessione improvvisa peer
                    if(ret == 0){
                        peerDisconnection(current_s);
                        continue;
                    }
                    
                    printf("%s: %s", sender, message);
                }
            }
        }
    }
}