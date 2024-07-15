// Segreteria.c
#include "../Source/lib.h"

int main() {

    // Alloca memoria per la struttura Thread_Segreteria, che contiene gli argomenti passati ai thread
    Thread_Segreteria *args = malloc(sizeof(Thread_Segreteria));

    // Dichiarazione delle variabili per i thread e il contatore
    pthread_t main_thread;      // Thread principale per gestire la connessione studente
    pthread_t input_thread;     // Thread per gestire l'input dalla segreteria
    int counter = 0;            // Contatore per gli ID assegnati agli studenti
    args->counter = counter;    // Inizializzazione dell'argomento counter nella struttura args

    // Crea un nuovo socket
    const int segreteriafd = Socket();

    // Configurazione del socket della segreteria
    const struct sockaddr_in segreteria_address = Configura_Indirizzo(SEGRETERIA_PORT);

    // Riutilizzo indirizzo socket
    Sock_Options(segreteriafd);

    // Associa il socket all'indirizzo
    Bind(segreteriafd, segreteria_address);

    // Inizializzazione dei parametri della struttura args relativi al socket della segreteria
    args->segreteriafd = segreteriafd;
    args->segreteria_address = segreteria_address;

    // Effettua collegamento con il Server.c
    int serverfd;
    struct sockaddr_in server_address;
    Connessione_Client(&serverfd, &server_address, SERVER_PORT);

    // Inizializzazione dei parametri della struttura args relativi al socket del server
    args->serverfd = serverfd;
    args->server_address = server_address;

    // Variabili per gestire la connessione con lo studente
    struct sockaddr_in studente_address;
    socklen_t studente_length = sizeof(studente_address);

    // Avvio del thread per gestire l'input dalla segreteria
    pthread_create(&input_thread, NULL, Thread_Input_Segreteria, args);
    pthread_detach(input_thread);

    // In attesa di connessioni da parte degli studenti
    Listen(segreteriafd);
    while (1) {

        // Effettua collegamento con lo Studente.c
        const int studentefd = Accept(segreteriafd, (struct sockaddr *) &studente_address, &studente_length);

        // Incrementa il contatore degli studenti connessi
        counter++;

        // Aggiorna il contatore nella struttura e memorizza il socket dello studente
        args->counter = counter;
        args->studentefd = studentefd;
        printf("\nConnessione stabilita con un nuovo studente. | ID assegnato: %d.\n", counter);

        // Avvia un thread per gestire la connessione dello studente
        pthread_create(&main_thread, NULL, Thread_Gestione_Studente, args);
        pthread_detach(main_thread);
    }
}
