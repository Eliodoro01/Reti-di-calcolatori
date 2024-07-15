// Segreteria_Helper.c
#include "../Source/lib.h"

// Funzione thread per la gestione delle operazioni dello studente
void *Thread_Gestione_Studente(void *arg) {

    // Cast dell'argomento al tipo corretto
    const Thread_Segreteria *args = arg;

    // Estrazione dei parametri necessari dalla struttura args
    const int studentefd = args->studentefd;
    struct sockaddr_in segreteria_address = args->segreteria_address;
    const int serverfd = args->serverfd;
    struct sockaddr_in server_address = args->server_address;
    const int counter = args->counter;

    // Loop infinito per gestire le comunicazioni con lo studente
    while (1) {
        printf("\nIn attesa di richieste dallo studente %d.\n", counter);

        // Ricezione dei dati inviati dallo studente
        char *dati_studente = Receive(studentefd, &segreteria_address);
        if (strlen(dati_studente) == 0) {
            printf("Lo studente %d ha chiuso la connessione.\n", counter);

            // Chiude il socket dello studente
            Close(studentefd);

            // Termina il thread
            return NULL;
        }
        printf("\nRicezione dei dati dello studente %d.    | %s\n", counter, dati_studente);

        // Invio dei dati ricevuti al server
        Sendto(serverfd, dati_studente, server_address);
        printf("Invio dei dati al server.               | %s\n", dati_studente);

        // Ricezione della risposta dal server
        char *risposta_server = Receive(serverfd, &server_address);
        if (strlen(risposta_server) == 0) {
            printf("Il server ha chiuso la connessione.\n");

            // Chiude il socket del server
            Close(serverfd);

            // Termina il thread
            return NULL;
        }
        printf("Ricezione dei dati dal server.          | %s\n", risposta_server);

        // Invio della risposta ricevuta allo studente
        Sendto(studentefd, risposta_server, segreteria_address);
        printf("Invio dei dati allo studente %d.         | %s\n", counter, risposta_server);

        // Liberazione della memoria allocata per i dati
        free(dati_studente);
        free(risposta_server);
    }
}

// Funzione thread per gestire l'input dalla segreteria
void *Thread_Input_Segreteria(void *arg) {
    while (1) {

        // Ottiene l'opzione selezionata dalla segreteria
        const int input = Selezione_Richiesta_Segreteria();
        if (input == 3) {

            // Gestisce la richiesta di aggiunta di un esame
            Richiesta_Aggiunta_Esame(arg);
        } else if (input == 4) {

            // Gestisce la richiesta di aggiunta di un appello
            Richiesta_Aggiunta_Appello(arg);
        }
    }
}

// Funzione per permettere alla segreteria di selezionare un'operazione
int Selezione_Richiesta_Segreteria() {
    int option;

    // Mostra le opzioni disponibili per la segreteria
    printf("\nLISTA OPERAZIONI SEGRETERIA:\n");
    printf("3) Aggiungi Esame;\n");
    printf("4) Aggiungi Appello;\n");
    printf("Scegli un'opzione.\n\n");

    // Legge l'opzione selezionata dall'utente
    scanf("%d", &option);

    // Pulisce il buffer di input
    Clear_Input_Buffer();

    // Restituisce l'opzione selezionata
    return option;
}

// Funzione per gestire la richiesta di aggiunta di un esame da parte della segreteria
void Richiesta_Aggiunta_Esame(const void *arg) {

    // Cast dell'argomento al tipo corretto
    const Thread_Segreteria *args = arg;

    // Estrazione dei parametri necessari dalla struttura args
    const int serverfd = args->serverfd;
    struct sockaddr_in server_address = args->server_address;
    char buffer[MALLOC];

    // Legge il Nome dell'esame
    char nome_esame[50];
    printf("\nInserisci il nome dell'esame da aggiungere: ");
    if (fgets(nome_esame, sizeof(nome_esame), stdin) != NULL) {

        // Rimuove il newline dal nome dell'esame
        nome_esame[strcspn(nome_esame, "\n")] = 0;
    }

    // Legge i CFU dell'esame
    int cfu;
    printf("Inserisci i CFU dell'esame: ");
    scanf("%d", &cfu);
    Clear_Input_Buffer();

    // Legge il Corso di Studi dell'esame dell'esame
    char corso_di_studi[50];
    printf("Inserisci il corso di studi dell'esame: ");
    if (fgets(corso_di_studi, sizeof(corso_di_studi), stdin) != NULL) {

        // Rimuove il newline dal corso di studi
        corso_di_studi[strcspn(corso_di_studi, "\n")] = 0;
    }

    // Formatta i dati dell'esame da inviare al server
    snprintf(buffer, sizeof(buffer), "3;%s;%d;%s;", nome_esame, cfu, corso_di_studi);

    // Invia i dati al server
    Sendto(serverfd, buffer, server_address);

    // Riceve la conferma dal server sull'operazione
    *buffer = *Receive(serverfd, &server_address);
    if (strcmp(buffer, "SUCCESS") == 0) {
        printf("L'esame è stato aggiunto con successo.\n");
    } else if (strcmp(buffer, "FAILURE") == 0) {
        printf("Si è verificato un errore durante l'aggiunta dell'esame.\n");
    }
}

// Funzione per gestire la richiesta di aggiunta di un appello da parte della segreteria
void Richiesta_Aggiunta_Appello(const void *arg) {

    // Cast dell'argomento al tipo corretto
    const Thread_Segreteria *args = arg;

    // Estrazione dei parametri necessari dalla struttura args
    const int serverfd = args->serverfd;
    struct sockaddr_in server_address = args->server_address;
    char buffer[MALLOC];

    // Legge l'ID dell'esame
    int id_esame;
    printf("\nInserisci l'ID dell'esame per l'appello: ");
    scanf("%d", &id_esame);
    Clear_Input_Buffer();

    // Legge il Nome dell'appello
    char nome_appello[50];
    printf("Inserisci il nome dell'appello: ");
    if (fgets(nome_appello, sizeof(nome_appello), stdin) != NULL) {

        // Rimuove il newline dal nome dell'appello
        nome_appello[strcspn(nome_appello, "\n")] = 0;
    }

    // Legge la Data dell'appello
    char data_appello[11];
    printf("Inserisci la data dell'appello (formato YYYY-MM-DD): ");
    if (fgets(data_appello, sizeof(data_appello), stdin) != NULL) {
        data_appello[strcspn(data_appello, "\n")] = 0; // Rimuove il newline dalla data dell'appello
    }

    // Formatta i dati dell'appello da inviare al server
    snprintf(buffer, sizeof(buffer), "4;%d;%s;%s;", id_esame, nome_appello, data_appello);

    // Invia i dati al server
    Sendto(serverfd, buffer, server_address);

    // Riceve la conferma dal server sull'operazione
    *buffer = *Receive(serverfd, &server_address);
    if (strcmp(buffer, "SUCCESS") == 0) {
        printf("L'appello è stato aggiunto con successo.\n\n");
    } else if (strcmp(buffer, "FAILURE") == 0) {
        printf("Si è verificato un errore durante l'aggiunta dell'appello.\n\n");
    }
}
