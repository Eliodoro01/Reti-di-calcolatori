// Server.c
#include "../Source/lib.h"

int main() {

    // Crea il socket del server
    const int serverfd = Socket();

    // Configura l'indirizzo del server
    const struct sockaddr_in server_address = Configura_Indirizzo(SERVER_PORT);

    // Riutilizzo indirizzo socket
    Sock_Options(serverfd);

    // Associa il socket all'indirizzo
    Bind(serverfd, server_address);

    // Struttura per l'indirizzo della segreteria
    struct sockaddr_in segreteria_address;

    // Lunghezza dell'indirizzo della segreteria
    socklen_t segreteria_length = sizeof(segreteria_address);

    // Connessione al database SQLite
    sqlite3 *db = ConnessioneDB();

    // Mette il server in ascolto di connessioni in entrata
    Listen(serverfd);

    // Ciclo principale per gestire le connessioni in entrata
    while (1) {

        // Effettua collegamento con la Segreteria.c
        const int segreteriafd = Accept(serverfd, (struct sockaddr *) &segreteria_address, &segreteria_length);
        printf("ACCEPT: Connessione stabilita con la segreteria.\n");

        // Ciclo interno per gestire le richieste della segreteria
        while (1) {

            // Messaggio di attesa di richieste
            printf("\nIn attesa di richieste dalla segreteria.\n");

            // Riceve un messaggio dalla segreteria
            char *buffer = Receive(segreteriafd, &segreteria_address);
            if (strlen(buffer) == 0) {

                // Messaggio se la segreteria chiude la connessione
                printf("La segreteria ha chiuso la connessione.\n");

                // Chiude la connessione con la segreteria
                Close(segreteriafd);

                // Esce dal ciclo interno
                break;
            }

            // Copia il messaggio ricevuto in una variabile
            char *richiesta = strdup(buffer);

            // Estrae il token dalla richiesta
            const char *token = Estrai_Token(&buffer);

            if (token != NULL) {

                // Converte il token in intero
                const int option = atoi(token);

                // Switch per gestire le diverse opzioni ricevute dalla segreteria
                switch (option) {
                    case 0:

                        // Funzione per verificare le credenziali
                        Verifica_Credenziali(db, segreteriafd, segreteria_address, richiesta);
                        break;
                    case 1:

                        // Funzione per visualizzare gli appelli
                        Visualizza_Appelli(db, segreteriafd, segreteria_address, richiesta);
                        break;
                    case 2:

                        // Funzione per aggiungere una prenotazione
                        Aggiungi_Prenotazione(db, segreteriafd, segreteria_address, richiesta);
                        break;
                    case 3:

                        // Funzione per aggiungere un esame
                        Aggiungi_Esame(db, segreteriafd, segreteria_address, richiesta);
                        break;
                    case 4:

                        // Funzione per aggiungere un appello
                        Aggiungi_Appello(db, segreteriafd, segreteria_address, richiesta);
                        break;
                    default:

                        // Messaggio se l'opzione non è valida
                        fprintf(stderr, "Opzione non valida: %d\n", option);
                        break;
                }
            } else {
                // Messaggio in caso di errore nel parsing del messaggio
                fprintf(stderr, "Errore nel parsing del messaggio\n");
            }
        }
    }
}
