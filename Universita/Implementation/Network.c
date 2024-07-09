// Network.c
#include "../Source/lib.h"

// Funzione per creare un socket TCP IPv4
int Socket() {

    // Crea un socket TCP
    const int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {

        // Gestisce l'errore se la creazione del socket fallisce
        Error("Errore nella creazione del socket");
    }
    // Restituisce il descrittore del socket creato
    return socketfd;
}

// Funzione per impostare le opzioni del socket (riutilizzo dell'indirizzo)
void Sock_Options(const int socketfd) {
    const int option = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {

        // Gestisce l'errore se l'impostazione delle opzioni fallisce
        Error("Errore durante il riutilizzo dell'indirizzo");
    }
}

// Funzione per collegare il socket a un indirizzo specificato
void Bind(const int socketfd, struct sockaddr_in address) {
    if (bind(socketfd, (struct sockaddr *) &address, sizeof(address)) < 0) {

        // Gestisce l'errore se il bind del socket fallisce
        Error("Errore nel bind del socket");
    }
}

// Funzione per mettere il socket in ascolto di connessioni in entrata
int Listen(const int socketfd) {
    const int max_connections = 10;
    if (listen(socketfd, max_connections) < 0) {

        // Gestisce l'errore se la listen del socket fallisce
        Error("Errore nella listen del socket");
    }
    printf("LISTEN: Il canale è in ascolto.\n");
    return 0;
}

// Funzione per configurare un indirizzo IPv4 con il port specificato
struct sockaddr_in Configura_Indirizzo(const int port) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;                   // Utilizza IPv4
    address.sin_addr.s_addr = htonl(INADDR_ANY);    // Accetta connessioni da qualsiasi interfaccia di rete
    address.sin_port = htons(port);                 // Imposta il numero di porta

    printf("\nCONFIG: Eseguita configurazione indirizzo sul port: %d.\n", port);

    // Restituisce l'indirizzo configurato
    return address;
}

// Funzione per accettare una connessione in entrata
int Accept(const int socketfd, struct sockaddr *address, socklen_t *adress_length) {

    // Accetta la connessione in entrata
    const int connfd = accept(socketfd, address, adress_length);
    if (connfd < 0) {

        // Gestisce l'errore se l'accettazione della connessione fallisce
        perror("Errore durante l'accettazione della connessione");
        return -1;
    }

    // Restituisce il descrittore del socket della nuova connessione
    return connfd;
}

// Funzione per chiudere il socket
void Close(const int socketfd) {
    if (close(socketfd) < 0) {

        // Gestisce l'errore se la chiusura del socket fallisce
        Error("Errore nella chiusura del socket");
    }
}

// Funzione per inviare un messaggio a un indirizzo specificato
void Sendto(const int socketfd, const char *messaggio, struct sockaddr_in address) {

    // Buffer per contenere il messaggio
    char buffer[MALLOC];

    // Formatta il messaggio nel buffer
    sprintf(buffer, "%s", messaggio);

    // Invia il messaggio
    const ssize_t n = sendto(socketfd, buffer, strlen(buffer), 0, (struct sockaddr *) &address, sizeof(address));
    if (n < 0) {

        // Gestisce l'errore se l'invio del messaggio fallisce
        Error("Errore durante la spedizione");
    }
}

// Funzione per ricevere un messaggio da un socket
char *Receive(const int socketfd, struct sockaddr_in *address) {

    // Buffer per contenere il messaggio ricevuto
    char buffer[MALLOC];

    // Alloca memoria per il messaggio
    char *messaggio = malloc(MALLOC + 1);
    if (messaggio == NULL) {

        // Gestisce l'errore se l'allocazione di memoria fallisce
        Error("Errore nella allocazione di memoria");
    }
    socklen_t len = sizeof(*address);
    memset(buffer, 0, sizeof(buffer));

    // Riceve il messaggio
    const ssize_t n = recvfrom(socketfd, buffer, MALLOC, 0, (struct sockaddr *) address, &len);
    if (n < 0) {

        // Gestisce l'errore se la ricezione del messaggio fallisce
        Error("Errore durante la ricezione");
    }

    // Termina il buffer con null character
    buffer[n] = '\0';

    // Copia il messaggio nel buffer allocato
    strncpy(messaggio, buffer, MALLOC);

    // Restituisce il messaggio ricevuto
    return messaggio;
}

// Funzione per connettersi a un server remoto
void Connessione_Client(int *socketfd, struct sockaddr_in *server_address, const int port) {

    // Crea un socket
    *socketfd = Socket();

    // Configura l'indirizzo del server
    *server_address = Configura_Indirizzo(port);
    int tentativi = 5;

    if (inet_pton(AF_INET, MAIN_IP, &server_address->sin_addr) <= 0) {

        // Gestisce l'errore se la conversione dell'indirizzo IP fallisce
        Error("Errore nella conversione dell'indirizzo IP");
    }

    // Messaggio di inizio connessione
    printf("CONNECT: Inizio tentativi (%d) di connessione al server.\n", tentativi);
    while (connect(*socketfd, (struct sockaddr *) server_address, sizeof(*server_address)) < 0) {
        tentativi--;

        // Stampa il numero di tentativi rimanenti
        printf("Connessione fallita. Numero tentativi: %d\n", tentativi);
        if (tentativi == 0) {

            // Gestisce l'errore se i tentativi di connessione sono esauriti
            Error("Errore nella connessione al server");
        }

        // Attende prima di un nuovo tentativo
        sleep(3);
    }

    // Messaggio di connessione riuscita
    printf("CONNECT: Canale connesso con successo al server con %d tentativi rimanenti.\n", *socketfd);
}
