typedef int bool;
#define true 1
#define false 0

struct ChatState{
    // Variabili per chat P2P
    // i messaggi successivi al primo vengono mandati al peer al peer che per ultimo ha chattato con me
    // con primo messaggio si intende, primo messaggio dopo comando chat username
    // salvo il socket e l'username dell'ultimo peer con cui ho chattato
    bool chat_on; 
    int last_chat_sock;
    char last_chat_peer[1024];

    // Variabili gestione gruppo
    // quando creo un gruppo entrano subito 3 peer
    // 1) peer che digita nella chat \a username
    // 2) peer con cui stava chattando il peer 1
    // 3) peer con username digitato da peer 1 
    // ci può essere un solo gruppo alla volta, quando un peer esce dal gruppo, tutti i peer escono dal gruppo
    int last_port_chat_peer;
    //bool chat_gruppo_attiva = 0, num_componenti = 0;
    //int porte_componenti_gruppo[10]; // max componenti gruppo: 10
    //int socket_componenti[10]; // socket per comunicazione con ogni componente del gruppo
};