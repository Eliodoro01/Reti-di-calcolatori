---
title: Università
author: Francesco Maria Luisi
---

# Introduzione

Università è un progetto universitario di Reti di Calcolatori che consiste in un appicativo Client/Server.

## Traccia

Scrivere un'applicazione client/server parallelo per gestire gli esami universitari

Segreteria:

Inserisce gli esami sul server dell'università (salvare in un file o conservare in memoria il dato)
Inoltra la richiesta di prenotazione degli studenti al server universitario
Fornisce allo studente le date degli esami disponibili per l'esame scelto dallo studente

Studente:

Chiede alla segreteria se ci siano esami disponibili per un corso
Invia una richiesta di prenotazione di un esame alla segreteria
Server universitarioL:

Riceve l'aggiunta di nuovi esami
Riceve la prenOtazione di un esaMe


Il server universItario ad ogni richiesta di prenotazione invia alla segreteria il numero di prenotazione progressivo assegnato allo studente e la segreteria a sua volta lo inoltra allo studente 

# Dettagli Progetto


Network.c
```c
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

```

Network.c fornisce un set di funzioni per la gestione di socket TCP IPv4. Le funzionalità implementate includono la creazione, la configurazione, il binding, l'ascolto, l'accettazione di connessioni, l'invio e la ricezione di messaggi, la chiusura di socket e la connessione a un server remoto.

Funzionalità Specifiche

Creazione e Gestione Socket:
La funzione Socket() crea un socket TCP IPv4 e gestisce eventuali errori.
La funzione Close() chiude un socket e gestisce eventuali errori.
Configurazione Socket:
La funzione Sock_Options() imposta l'opzione SO_REUSEADDR per consentire il riutilizzo dell'indirizzo IP del socket.
La funzione Configura_Indirizzo() configura un indirizzo sockaddr_in con un numero di porta specificato.
Binding e Ascolto:
La funzione Bind() associa un socket a un indirizzo locale.
La funzione Listen() mette un socket in ascolto di connessioni in entrata.
Accettazione Connessioni:
La funzione Accept() accetta una connessione in entrata e restituisce il descrittore del socket della nuova connessione.
Invio e Ricezione Messaggi:
La funzione Sendto() invia un messaggio a un indirizzo specificato.
La funzione Receive() riceve un messaggio da un socket e lo restituisce come stringa.
Connessione a un Server Remoto:
La funzione Connessione_Client() crea un socket, configura l'indirizzo del server e si connette al server specificato.


Framework.c
```c
#include "../Source/lib.h"

// Funzione per pulire il buffer di input (rimuovere caratteri rimanenti dopo l'input)
void Clear_Input_Buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Continua a leggere caratteri fino a quando non si raggiunge una nuova linea o EOF
    }
}

// Funzione per gestire gli errori, stampando il messaggio di errore fornito
void Error(const char *msg) {

    // Stampa il messaggio di errore con perror
    perror(msg);

    // Esce dal programma con stato di errore
    exit(1);
}

// Funzione per estrarre un token dal puntatore ai dati, separati dal delimitatore ';'
char *Estrai_Token(char **dati) {

    // Estrae il token fino al prossimo ';'
    const char *token = strsep(dati, ";");
    if (token == NULL) {

        // Stampa un errore se il token è NULL
        fprintf(stderr, "ERROR: Token NULL\n");
        return NULL;
    }

    // Restituisce una copia allocata dinamicamente del token estratto
    return strdup(token);
}

// Funzione per connettersi al database SQLite
sqlite3 *ConnessioneDB() {
    sqlite3 *db;

    // Apre la connessione al database SQLite
    const int rc = sqlite3_open("identifier.sqlite", &db);
    if (rc) {

        // Stampa un errore se non riesce ad aprire il database
        fprintf(stderr, "Errore apertura database: %s\n", sqlite3_errmsg(db));

        // Esce dal programma con stato di errore
        exit(1);
    }
    printf("SQLITE: Connessione al database SQLite eseguita correttamente.\n");

    // Restituisce il puntatore alla connessione al database
    return db;
}

// Funzione per eseguire una query SQL sul database SQLite
int Esegui_Query(sqlite3 *db, const char *sql, sqlite3_stmt **sql_query) {

    // Prepara la query SQL
    const int sql_result = sqlite3_prepare_v2(db, sql, -1, sql_query, NULL);
    if (sql_result != SQLITE_OK) {

        // Stampa un errore se la preparazione della query fallisce
        fprintf(stderr, "Errore SQLite: %s\n", sqlite3_errmsg(db));

        // Restituisce il risultato dell'operazione SQLite
        return sql_result;
    }
    // Restituisce OK se la preparazione della query è andata a buon fine
    return SQLITE_OK;
}

```

Framework.c implementa diverse funzionalità di base per la gestione di input, la gestione degli errori, l'estrazione di token e l'interazione con un database SQLite in linguaggio C. Le funzioni incluse gestiscono la pulizia del buffer di input, la stampa di messaggi di errore, l'estrazione di token da stringhe delimitate, la connessione e l'esecuzione di query su un database SQLite.

Funzionalità Specifiche

Gestione Input:
La funzione Clear_Input_Buffer() rimuove i caratteri rimanenti nel buffer di input dopo l'acquisizione di un valore.
Gestione Errori:
La funzione Error() stampa un messaggio di errore generico e termina il programma con stato di errore.
Estrazione Token:
La funzione Estrai_Token() estrae un token da una stringa delimitata da ';' e restituisce una copia allocata dinamicamente del token estratto.
Interazione con Database SQLite:
La funzione ConnessioneDB() apre una connessione al database SQLite "identifier.sqlite" e restituisce il puntatore alla connessione.
La funzione Esegui_Query() prepara ed esegue una query SQL sul database SQLite specificato e restituisce il risultato dell'operazione.

Segreteria_Helper.c
```c
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

```
Segreteria_Helper.c implementa diverse funzionalità per la gestione delle comunicazioni tra la segreteria e un server, la gestione di richieste da parte degli studenti e l'esecuzione di operazioni su un database. Le funzioni incluse gestiscono la creazione di thread per la gestione di studenti e input dalla segreteria, la selezione di operazioni, l'invio e la ricezione di dati dal server, la formattazione di messaggi e l'esecuzione di richieste specifiche come l'aggiunta di esami e appelli.

Funzionalità Specifiche

Gestione Thread:
La funzione Thread_Gestione_Studente() crea un thread dedicato alla gestione delle comunicazioni con un singolo studente.
La funzione Thread_Input_Segreteria() crea un thread dedicato all'acquisizione di input dalla segreteria e all'avvio delle relative funzioni.
Comunicazione con il Server:
Le funzioni Sendto() e Receive() gestiscono l'invio e la ricezione di dati dal server tramite socket UDP.
I dati inviati al server sono formattati in stringhe con un separatore specifico (';') per identificare il tipo di operazione e i parametri.
Le risposte dal server sono ricevute e analizzate per determinare l'esito dell'operazione e comunicare il risultato alla segreteria o allo studente.
Gestione Richieste Segreteria:
La funzione Selezione_Richiesta_Segreteria() presenta un menu alla segreteria e acquisisce la sua scelta per l'operazione da eseguire.
Le funzioni Richiesta_Aggiunta_Esame() e Richiesta_Aggiunta_Appello() gestiscono le richieste specifiche per l'aggiunta di esami e appelli, acquisendo i dati necessari dall'utente, formattandoli e inviandoli al server.
Funzionalità Database (Implementazione non presente):
Si presume che il codice interagisca con un database per l'archiviazione o il recupero di dati relativi a esami, appelli e studenti.
L'interazione con il database non è implementata nel codice fornito, ma dovrebbe essere integrata per la persistenza dei dati.


Server_Helper.c
```c
#include "../Source/lib.h"

// Funzione per Verificare le Credenziali nel database
void Verifica_Credenziali(sqlite3 *db, const int segreteriafd, const struct sockaddr_in segreteria_address, char *dati) {

    // Estrai Matricola e Password dai dati ricevuti
    strsep(&dati, ";");
    char *matricola = Estrai_Token(&dati);
    char *password = Estrai_Token(&dati);

    // Stampare le credenziali ricevute (debug)
    printf("Credenziali ricevute:\nMatricola: %s | Password %s\n", matricola, password);

    // Costruzione della query SQL per verificare le credenziali
    char sql_query_text[MALLOC];
    snprintf(sql_query_text, sizeof(sql_query_text),"SELECT * FROM STUDENTE WHERE MATRICOLA = '%s' AND PASSWORD = '%s';", matricola, password);

    // Preparazione dello statement SQL
    sqlite3_stmt *prepared_statement;
    int sqlite_result_code = sqlite3_prepare_v2(db, sql_query_text, -1, &prepared_statement, NULL);
    if (sqlite_result_code != SQLITE_OK) {

        // Invia un messaggio di errore alla segreteria
        Sendto(segreteriafd, "Errore query credenziali\n", segreteria_address);
        fprintf(stderr, "Errore SQLite: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(prepared_statement);
        return;
    }

    // Esecuzione della query SQL
    sqlite_result_code = sqlite3_step(prepared_statement);

    // Feedback dell'esito della query
    printf("Query SQL per verificare le credenziali eseguita correttamente.\n");

    // Gestione dei risultati della query
    if (sqlite_result_code == SQLITE_ROW) {

        // Se le credenziali sono corrette, invia SUCCESS e il piano di studi
        char *piano_di_studi = strdup((char *) sqlite3_column_text(prepared_statement, 3));
        char buffer[MALLOC];
        snprintf(buffer, sizeof(buffer), "SUCCESS;%s", piano_di_studi);
        Sendto(segreteriafd, buffer, segreteria_address);
        printf("Il login dello studente %s è andato a buon fine ed ha ora accesso al server.\n", matricola);
    } else if (sqlite_result_code == SQLITE_DONE) {

        // Se le credenziali non sono corrette, invia FAILURE
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        printf("Il login dello studente %s è fallito in quanto le credenziali non coincidono.\n", matricola);
    } else {

        // Gestione degli altri errori SQLite
        fprintf(stderr, "Errore SQLite: %s\n", sqlite3_errmsg(db));
    }

    // Finalizzazione dello statement SQL
    sqlite3_finalize(prepared_statement);
}

// Funzione per visualizzare gli appelli nel database
void Visualizza_Appelli(sqlite3 *db, const int segreteriafd, const struct sockaddr_in segreteria_address, char *dati) {

    // Estrai il piano di studi dai dati ricevuti
    strsep(&dati, ";");
    char *piano_di_studi = Estrai_Token(&dati);

    // Costruzione della query SQL per visualizzare gli appelli
    char sql_query_text[MALLOC];
    sprintf(sql_query_text,"SELECT ESAME.NOME_ESAME, APPELLO.NOME_APPELLO, APPELLO.DATA_APPELLO, ESAME.ID_ESAME, APPELLO.ID_APPELLO FROM APPELLO JOIN ESAME ON APPELLO.ID_ESAME = ESAME.ID_ESAME WHERE ESAME.CORSO_DI_STUDI = '%s';", piano_di_studi);
    sqlite3_stmt *prepared_statement;

    // Preparazione e esecuzione della query SQL
    if (sqlite3_prepare_v2(db, sql_query_text, -1, &prepared_statement, NULL) != SQLITE_OK) {

        // In caso di errore, invia un messaggio di errore alla segreteria
        Sendto(segreteriafd, "Errore query appelli", segreteria_address);
        fprintf(stderr, "Errore SQLite: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Feedback sull'esecuzione della query
    printf("Query SQL per reperire gli appelli eseguita correttamente.\n");

    // Costruzione del buffer per i risultati della query
    char buffer[MALLOC] = "";
    while (sqlite3_step(prepared_statement) == SQLITE_ROW) {

        // Recupera i dati degli appelli
        const char *nome_esame = (const char *) sqlite3_column_text(prepared_statement, 0);
        const char *nome_appello = (const char *) sqlite3_column_text(prepared_statement, 1);
        const char *data_appello = (const char *) sqlite3_column_text(prepared_statement, 2);
        const char *id_esame = (const char *) sqlite3_column_text(prepared_statement, 3);
        const char *id_appello = (const char *) sqlite3_column_text(prepared_statement, 4);

        // Costruzione del buffer di risposta
        strcat(buffer, nome_esame);
        strcat(buffer, ";");
        strcat(buffer, id_esame);
        strcat(buffer, ";");
        strcat(buffer, nome_appello);
        strcat(buffer, ";");
        strcat(buffer, data_appello);
        strcat(buffer, ";");
        strcat(buffer, id_appello);
        strcat(buffer, ";");
    }

    // Stampa del contenuto del buffer (debug)
    printf("Contenuto del buffer: %s\n", buffer);

    // Invio dei dati alla segreteria
    Sendto(segreteriafd, buffer, segreteria_address);

    // Finalizzazione dello statement SQL
    sqlite3_finalize(prepared_statement);

    // Feedback dell'operazione completata
    printf("Appelli esami reperiti e spediti correttamente.\n\n");

    // Liberazione della memoria allocata per il piano di studi
    free(piano_di_studi);
}

// Funzione per aggiungere una Prenotazione nel database
void Aggiungi_Prenotazione(sqlite3 *db, const int segreteriafd, const struct sockaddr_in segreteria_address, char *dati) {

    // Estrai la Matricola, l'ID dell'esame e l'ID dell'appello dai dati ricevuti
    strsep(&dati, ";");
    char *matricola = Estrai_Token(&dati);
    char *id_esame = Estrai_Token(&dati);
    char *id_appello = Estrai_Token(&dati);

    // Stampa dei dati di prenotazione ricevuti (debug)
    printf("Dati di prenotazione ricevuti:\nMatricola: %s | ID Esame: %s | ID Appello: %s\n", matricola, id_esame, id_appello);

    // Verifica se l'esame è associato al corso di studi dello studente
    char sql_check_course[MALLOC];
    sprintf(sql_check_course,"SELECT COUNT(*) FROM STUDENTE AS S JOIN ESAME AS E ON S.PIANO_DI_STUDI = E.CORSO_DI_STUDI WHERE S.MATRICOLA = %s AND E.ID_ESAME = %s;", matricola, id_esame);

    int count_course = 0;
    sqlite3_stmt *stmt_course;
    if (sqlite3_prepare_v2(db, sql_check_course, -1, &stmt_course, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt_course) == SQLITE_ROW) {
            count_course = sqlite3_column_int(stmt_course, 0);
        }
    }
    sqlite3_finalize(stmt_course);

    // Se l'esame non è associato al corso di studi, invia un messaggio di errore
    if (count_course == 0) {
        Sendto(segreteriafd, "INVALID COURSE", segreteria_address);
        printf("La prenotazione è fallita perché l'esame non è associato al corso di studi dello studente.\n");
    }

    // Verifica se l'appello è valido per l'esame specificato
    char sql_check_appello[MALLOC];
    sprintf(sql_check_appello, "SELECT COUNT(*) FROM APPELLO WHERE ID_APPELLO = %s AND ID_ESAME = %s;", id_appello, id_esame);
    int count_appello = 0;
    sqlite3_stmt *stmt_appello;
    if (sqlite3_prepare_v2(db, sql_check_appello, -1, &stmt_appello, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt_appello) == SQLITE_ROW) {
            count_appello = sqlite3_column_int(stmt_appello, 0);
        }
    }
    sqlite3_finalize(stmt_appello);

    // Se l'appello non è valido, invia un messaggio di errore
    if (count_appello == 0) {
        Sendto(segreteriafd, "APPELLO INVALIDO", segreteria_address);
        printf("La prenotazione è fallita perché l'ID dell'appello non corrisponde all'ID dell'esame inserito.\n");
    }

    // Verifica se lo studente ha già prenotato per quell'appello
    char sql_check[MALLOC];
    sprintf(sql_check, "SELECT COUNT(*) FROM PRENOTAZIONE WHERE MATRICOLA = %s AND ID_APPELLO = %s;", matricola, id_appello);
    int count = 0;
    sqlite3_stmt *stmt_check;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt_check) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt_check, 0);
        }
    }
    sqlite3_finalize(stmt_check);

    // Se lo studente ha già prenotato per quell'appello, invia un messaggio di errore
    if (count > 0) {
        Sendto(segreteriafd, "ALREADY THERE", segreteria_address);
        printf("La prenotazione è fallita in quanto lo studente ha già prenotato per quell'appello.\n");
    } else {

        // Se tutte le verifiche passano, aggiungi la prenotazione
        char sql_insert[MALLOC];
        sprintf(sql_insert,"INSERT INTO PRENOTAZIONE (MATRICOLA, ID_APPELLO, DATA_PRENOTAZIONE) VALUES (%s, %s, DATE('now'));", matricola, id_appello);

        char *errore;
        const int rc = sqlite3_exec(db, sql_insert, 0, 0, &errore);
        if (rc != SQLITE_OK) {

            // In caso di errore durante l'inserimento, invia un messaggio di errore
            fprintf(stderr, "Errore SQLite: %s\n", errore);
            sqlite3_free(errore);
        } else {

            // Se l'inserimento va a buon fine, invia SUCCESS e l'ID di prenotazione generato
            const sqlite3_int64 id_prenotazione = sqlite3_last_insert_rowid(db);
            char msg[MALLOC];
            sprintf(msg, "SUCCESS;%lld", id_prenotazione);
            Sendto(segreteriafd, msg, segreteria_address);
            printf("La prenotazione dell'appello %s da parte dello studente %s è stata eseguita con successo.\n", id_appello, matricola);
            printf("Generato ID di prenotazione: %lld\n\n", id_prenotazione);
        }
    }

    // Liberazione della memoria allocata
    free(matricola);
    free(id_esame);
    free(id_appello);
}

// Funzione per aggiungere un Esame nel database
void Aggiungi_Esame(sqlite3 *db, const int segreteriafd, const struct sockaddr_in segreteria_address, char *dati) {

    // Estrai il Nome dell'esame dai dati ricevuti
    strsep(&dati, ";");
    char *nome_esame = Estrai_Token(&dati);
    if (nome_esame == NULL) {
        fprintf(stderr, "Errore: nome_esame è NULL\n");
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Estrai il CFU dell'esame dai dati ricevuti
    const int cfu = atoi(Estrai_Token(&dati));
    if (cfu <= 0) {
        fprintf(stderr, "Errore: CFU non valido\n");
        free(nome_esame);
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Estrai il Corso di Studi dell'esame dai dati ricevuti
    char *corso_di_studi = Estrai_Token(&dati);
    if (corso_di_studi == NULL) {
        fprintf(stderr, "Errore: corso_di_studi è NULL\n");
        free(nome_esame);
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Stampa dei dati dell'esame ricevuti (debug)
    printf("Dati dell'esame ricevuti:\nNome Esame: %s | CFU: %d | Corso di Studi: %s\n", nome_esame, cfu, corso_di_studi);

    // Costruzione della query SQL per aggiungere l'esame
    char sql_insert[MALLOC];
    snprintf(sql_insert, sizeof(sql_insert),"INSERT INTO ESAME (NOME_ESAME, CFU, CORSO_DI_STUDI) VALUES ('%s', %d, '%s');", nome_esame, cfu, corso_di_studi);

    // Esecuzione della query SQL
    char *errore;
    const int rc = sqlite3_exec(db, sql_insert, 0, 0, &errore);
    if (rc != SQLITE_OK) {

        // In caso di errore, stampa un messaggio di errore e invia FAILURE alla segreteria
        fprintf(stderr, "Errore SQLite: %s\n", errore);
        sqlite3_free(errore);
        printf("DEBUG: Invio risposta di errore alla segreteria...\n");
        Sendto(segreteriafd, "FAILURE", segreteria_address);
    } else {

        // Se l'inserimento va a buon fine, stampa un messaggio di successo e invia SUCCESS alla segreteria
        printf("L'esame è stato aggiunto con successo.\n");
        printf("DEBUG: Invio risposta di successo alla segreteria...\n");
        Sendto(segreteriafd, "SUCCESS", segreteria_address);
    }

    // Liberazione della memoria allocata
    free(nome_esame);
}

// Funzione per aggiungere un Appello nel database
void Aggiungi_Appello(sqlite3 *db, const int segreteriafd, const struct sockaddr_in segreteria_address, char *dati) {

    // Estrai l'ID dell'esame dai dati ricevuti
    strsep(&dati, ";");
    const int id_esame = atoi(Estrai_Token(&dati));
    if (id_esame <= 0) {
        fprintf(stderr, "Errore: ID esame non valido\n");
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Estrai il Nome dai dati ricevuti
    char *nome_appello = Estrai_Token(&dati);
    if (nome_appello == NULL) {
        fprintf(stderr, "Errore: nome_appello è NULL\n");
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Estrai la Data dai dati ricevuti
    char *data_appello = Estrai_Token(&dati);
    if (data_appello == NULL) {
        fprintf(stderr, "Errore: data_appello è NULL\n");
        free(nome_appello);
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Stampa dei dati dell'appello ricevuti (debug)
    printf("Dati dell'appello ricevuti:\nID Esame: %d | Nome Appello: %s | Data Appello: %s\n", id_esame, nome_appello, data_appello);

    // Query per ottenere l'ID successivo per l'appello
    char *errore;
    char sql_query[MALLOC];
    snprintf(sql_query, sizeof(sql_query), "SELECT MAX(ID_APPELLO) FROM APPELLO;");
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0) != SQLITE_OK) {

        // In caso di errore, invia un messaggio di errore alla segreteria
        fprintf(stderr, "Errore SQLite: %s\n", sqlite3_errmsg(db));
        Sendto(segreteriafd, "FAILURE", segreteria_address);
        return;
    }

    // Ottieni il prossimo ID appello
    int id_appello = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_appello = sqlite3_column_int(stmt, 0) + 1;
    }
    sqlite3_finalize(stmt);

    // Costruzione della query SQL per aggiungere l'appello
    char sql_insert[MALLOC];
    snprintf(sql_insert, sizeof(sql_insert),"INSERT INTO APPELLO (ID_APPELLO, NOME_APPELLO, DATA_APPELLO, ID_ESAME) VALUES (%d, '%s', '%s', %d);", id_appello, nome_appello, data_appello, id_esame);

    // Esecuzione della query SQL
    const int rc = sqlite3_exec(db, sql_insert, 0, 0, &errore);
    if (rc != SQLITE_OK) {

        // In caso di errore, stampa un messaggio di errore e invia FAILURE alla segreteria
        fprintf(stderr, "Errore SQLite: %s\n", errore);
        sqlite3_free(errore);
        printf("DEBUG: Invio risposta di errore alla segreteria...\n");
        Sendto(segreteriafd, "FAILURE", segreteria_address);
    } else {

        // Se l'inserimento va a buon fine, stampa un messaggio di successo e invia SUCCESS alla segreteria
        printf("L'appello è stato aggiunto con successo.\n");
        printf("DEBUG: Invio risposta di successo alla segreteria...\n");
        Sendto(segreteriafd, "SUCCESS", segreteria_address);
    }
}

```

Server_Helper.c implementa diverse funzionalità per la gestione delle comunicazioni tra un server e un'applicazione client, in particolare con focus sulla gestione di richieste da parte degli studenti e l'esecuzione di operazioni su un database. Le funzioni incluse gestiscono la ricezione di dati dal client, la verifica delle credenziali degli utenti, l'esecuzione di query SQL su un database, la formattazione e l'invio di risposte al client.

Funzionalità Specifiche

Gestione delle Comunicazioni:
La funzione Receivefrom() riceve i dati dal client tramite socket UDP.
La funzione Sendto() invia dati al client tramite socket UDP.
I dati inviati e ricevuti sono formattati con un separatore specifico (';') per identificare i comandi e i parametri.
Verifica Credenziali:
La funzione Verifica_Credenziali() controlla le credenziali di un utente (matricola e password) nel database e invia un messaggio di successo o di fallimento al client.
Visualizzazione Appelli:
La funzione Visualizza_Appelli() recupera i dati sugli appelli disponibili per un determinato corso di studi dal database e li invia al client.
Aggiunta Prenotazione:
La funzione Aggiungi_Prenotazione() inserisce una nuova prenotazione per un appello da parte di uno studente nel database, verificando la validità dei dati e inviando un messaggio di successo o di fallimento al client.
Aggiunta Esame:
La funzione Aggiungi_Esame() inserisce un nuovo esame nel database, controllando la validità dei dati e inviando un messaggio di successo o di fallimento al client.
Aggiunta Appello:
La funzione Aggiungi_Appello() inserisce un nuovo appello per un esame esistente nel database, controllando la validità dei dati e inviando un messaggio di successo o di fallimento al client.

Studente_Helper.c
```c
#include "../Source/lib.h"

// Funzione per effettuare il login dello studente
Studente *Login(int *studentefd, struct sockaddr_in *segreteria_address) {

    // Inizializza la connessione del client con la segreteria
    Connessione_Client(studentefd, segreteria_address, SEGRETERIA_PORT);

    // Alloca memoria per la struttura dati dello studente
    Studente *studente = malloc(sizeof(Studente));

    // Legge la Matricola dello studente
    printf("\nInserisci la tua matricola: ");
    fgets(studente->matricola, sizeof(studente->matricola), stdin);
    studente->matricola[strcspn(studente->matricola, "\n")] = '\0';

    // Legge la Password dello studente
    char password[50];
    printf("Inserisci la tua password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    // Costruisce il messaggio da inviare al server della segreteria per il login
    char buffer[MALLOC];
    snprintf(buffer, sizeof(buffer), "0;%s;%s;", studente->matricola, password);

    // Invia il messaggio di login al server della segreteria
    Sendto(*studentefd, buffer, *segreteria_address);

    // Riceve la risposta dal server della segreteria
    char *risposta = Receive(*studentefd, segreteria_address);

    // Analizza la risposta per determinare l'esito del login
    const char *token = strtok(risposta, ";");
    if (strcmp(token, "SUCCESS") == 0) {
        token = strtok(NULL, ";");
        strncpy(studente->piano_di_studi, token, sizeof(studente->piano_di_studi) - 1);
        studente->piano_di_studi[sizeof(studente->piano_di_studi) - 1] = '\0';
        free(risposta);

        // Restituisce la struttura dati dello studente dopo il login
        return studente;
    }

    // Dealloca memoria e restituisce NULL in caso di fallimento del login
    free(risposta);
    free(studente);
    return NULL;
}

// Funzione per la selezione dell'operazione da parte dello studente
int Selezione_Richiesta_Studente() {
    int option;
    printf("\nLISTA OPERAZIONI STUDENTE:\n");
    printf("1) Visualizza appelli;\n");
    printf("2) Prenotazione appello;\n");
    printf("0) Esci;\n");
    printf("Scegli un'opzione: ");

    // Legge l'opzione selezionata dall'utente
    scanf("%d", &option);

    // Pulisce il buffer di input
    Clear_Input_Buffer();

    // Restituisce l'opzione scelta dall'utente
    return option;
}

// Funzione per la richiesta di visualizzazione degli appelli disponibili
void Richiesta_Visualizzazione_Appelli(const int studentefd, struct sockaddr_in segreteria_address, char *piano_di_studi) {
    char buffer[MALLOC];

    // Costruisce il messaggio da inviare al server
    snprintf(buffer, sizeof(buffer), "1;%s;", piano_di_studi);

    // Invia la richiesta di visualizzazione degli appelli al server della segreteria
    Sendto(studentefd, buffer, segreteria_address);

    // Riceve la risposta dal server della segreteria
    char *risultato = Receive(studentefd, &segreteria_address);
    char *token = strtok(risultato, ";");

    // Itera attraverso i token ricevuti e stampa i dettagli degli appelli
    while (token != NULL) {
        char *nome_esame = token;
        token = strtok(NULL, ";");
        if (token == NULL) break;

        char *id_esame = token;
        token = strtok(NULL, ";");
        if (token == NULL) break;

        char *nome_appello = token;
        token = strtok(NULL, ";");
        if (token == NULL) break;

        char *data_appello = token;
        token = strtok(NULL, ";");
        if (token == NULL) break;

        char *id_appello = token;
        token = strtok(NULL, ";");

        // Stampa i dettagli dell'esame e dell'appello
        printf("\nEsame: %s | ID Esame: %s\n", nome_esame, id_esame);
        printf("Appello: %s | Data: %s | ID Appello: %s\n", nome_appello, data_appello, id_appello);
    }

    // Libera la memoria allocata per la risposta
    free(risultato);
}

// Funzione per la richiesta di prenotazione di un appello
void Richiesta_Prenotazione_Appello(const int studentefd, struct sockaddr_in segreteria_address, char *matricola) {
    char id_esame[10];
    char id_appello[10];

    // Legge l'ID dell'esame desiderato
    printf("\nInserisci l'ID dell'esame a cui vuoi prenotarti: ");
    fgets(id_esame, 10, stdin);
    id_esame[strcspn(id_esame, "\n")] = 0;

    // Legge l'ID dell'appello desiderato
    printf("\nInserisci l'ID dell'appello a cui vuoi prenotarti: ");
    fgets(id_appello, 10, stdin);
    id_appello[strcspn(id_appello, "\n")] = 0;

    char buffer[MALLOC];
    snprintf(buffer, sizeof(buffer), "2;%s;%s;%s;", matricola, id_esame, id_appello); // Costruisce il messaggio da inviare al server

    // Invia la richiesta di prenotazione dell'appello al server della segreteria
    Sendto(studentefd, buffer, segreteria_address);

    // Riceve la risposta dal server della segreteria
    char *result = Receive(studentefd, &segreteria_address);

    // Gestisce la risposta ricevuta dal server della segreteria
    if (strncmp(result, "SUCCESS", 7) == 0) {
        char *id_prenotazione = result + 8;
        printf("\nLa prenotazione è andata a buon fine: ID Prenoggggtazione = %s.\n", id_prenotazione);
    } else if (strcmp(result, "ALREADY THERE") == 0) {
        printf("\nHai già effettuato una prenotazione su questo appello.\n");
    } else if (strcmp(result, "INVALID COURSE") == 0) {
        printf("\nNon puoi prenotarti a questo esame/appello perché non è associato al tuo corso di studi.\n");
    } else if (strcmp(result, "APPELLO INVALIDO") == 0) {
        printf("\nL'ID dell'appello non corrisponde all'ID dell'esame inserito.\n");
    }

    // Libera la memoria allocata per la risposta
    free(result);
}

```

Studente_Helper.c implementa diverse funzionalità per la gestione delle interazioni tra uno studente e un server di segreteria universitaria. Le funzioni incluse permettono allo studente di effettuare il login, selezionare un'operazione da compiere, visualizzare gli appelli disponibili per il suo corso di studi e prenotare un appello per un esame.

Funzionalità Specifiche

Login Studente:
La funzione Login() permette allo studente di inserire la propria matricola e password per autenticarsi sul server. In caso di login avvenuto con successo, la funzione restituisce una struttura dati "Studente" contenente informazioni come il piano di studi.
Selezione Operazione:
La funzione Selezione_Richiesta_Studente() presenta un menu all'utente con le opzioni disponibili (visualizzazione appelli, prenotazione appello, uscita). La funzione restituisce l'opzione scelta dall'utente.
Visualizzazione Appelli:
La funzione Richiesta_Visualizzazione_Appelli() invia al server una richiesta per ottenere l'elenco degli appelli disponibili per il corso di studi dello studente. Riceve la risposta dal server e stampa a video i dettagli di ogni appello (nome esame, ID esame, nome appello, data, ID appello).
Prenotazione Appello:
La funzione Richiesta_Prenotazione_Appello() permette allo studente di inserire l'ID di un esame e di un appello per prenotarsi. Invia la richiesta al server e riceve una risposta che indica l'esito della prenotazione (successo, prenotazione già esistente, corso di studi non valido, ID appello non valido).

Server.c
```c
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

```

Server.c implementa le funzionalità lato server per la gestione di un sistema di segreteria universitaria. Il server si mette in ascolto su una porta specifica, accetta connessioni da parte di client (studenti e segreteria) e gestisce le loro richieste. Le funzioni incluse permettono di verificare le credenziali degli utenti, visualizzare gli appelli disponibili, aggiungere prenotazioni, esami e appelli.

Funzionalità Specifiche

Gestione Connessioni:
Il server crea un socket e si associa a un indirizzo specifico.
Il server rimane in ascolto di connessioni in entrata da parte di client.
Il server accetta una connessione alla volta e la gestisce in un ciclo dedicato.
Il server riceve messaggi dai client, li analizza e invia risposte.
Verifica Credenziali:
La funzione Verifica_Credenziali() controlla le credenziali di un utente (matricola e password) nel database e invia un messaggio di successo o di fallimento al client.
Visualizzazione Appelli:
La funzione Visualizza_Appelli() recupera i dati sugli appelli disponibili per un determinato corso di studi dal database e li invia al client.
Aggiunta Prenotazione:
La funzione Aggiungi_Prenotazione() inserisce una nuova prenotazione per un appello da parte di uno studente nel database, verificando la validità dei dati e inviando un messaggio di successo o di fallimento al client.
Aggiunta Esame:
La funzione Aggiungi_Esame() inserisce un nuovo esame nel database, controllando la validità dei dati e inviando un messaggio di successo o di fallimento al client.
Aggiunta Appello:
La funzione Aggiungi_Appello() inserisce un nuovo appello per un esame esistente nel database, controllando la validità dei dati e inviando un messaggio di successo o di fallimento al client.

Segreteria.c
```c
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

```

Segreteria.c implementa le funzionalità lato segreteria per la gestione di un sistema di segreteria universitaria. Il programma crea un socket e si mette in ascolto su una porta specifica, accetta connessioni da parte degli studenti e gestisce le loro richieste tramite thread dedicati. Le funzioni incluse permettono di gestire connessioni multiple, assegnare un ID univoco a ciascun studente e avviare thread separati per la gestione dell'input dalla segreteria e delle interazioni con gli studenti.

Funzionalità Specifiche

Gestione Connessioni:
Il server crea un socket e si associa a un indirizzo specifico.
Il server rimane in ascolto di connessioni in entrata da parte degli studenti.
Il server accetta una connessione alla volta e la gestisce in un ciclo dedicato.
Il server riceve messaggi dai client, li analizza e invia risposte.
Assegnazione ID Studente:
Un contatore viene utilizzato per assegnare un ID univoco a ciascun studente che si connette.
L'ID viene memorizzato nella struttura dati "Thread_Segreteria" e passato ai thread dedicati.
Gestione Thread:
Un thread principale viene creato per gestire la connessione con ogni studente.
Un thread separato viene creato per gestire l'input dalla segreteria.
I thread utilizzano la sincronizzazione per accedere in modo sicuro alle risorse condivise.

Studente.c
```c
#include "../Source/lib.h"

int main() {
    int studentefd;                         // Descrittore del socket dello studente
    struct sockaddr_in segreteria_address;  // Indirizzo della segreteria

    // Loop principale del programma studente
    while (1) {

        // Effettua il login dello studente
        Studente *studente = Login(&studentefd, &segreteria_address);

        // Verifica se il login è stato eseguito con successo
        if (studente != NULL) {
            printf("SUCCESS: Il login ha avuto successo.\n");

            // Loop per gestire le operazioni dello studente
            while (1) {

                // Ottiene l'opzione selezionata dallo studente
                const int scelta = Selezione_Richiesta_Studente();

                // Gestisce la scelta effettuata dallo studente
                if (scelta == 1) {
                    Richiesta_Visualizzazione_Appelli(studentefd, segreteria_address, studente->piano_di_studi);
                } else if (scelta == 2) {
                    Richiesta_Prenotazione_Appello(studentefd, segreteria_address, studente->matricola);
                } else if (scelta == 0) {

                    // Chiude il socket dello studente
                    Close(studentefd);

                    // Libera la memoria allocata per lo studente
                    free(studente);

                    // Esce dal loop di gestione delle operazioni
                    break;
                } else {

                    // Gestisce opzioni non valide
                    printf("Opzione non valida.\n");
                }
            }

            // Esce dal loop principale del programma studente
            break;
        }

        // Gestisce il fallimento del login
        printf("FAILURE: Credenziali scorrette. Riprova\n");
    }
}

```

Studente.c implementa l'interfaccia lato studente per l'interazione con un sistema di segreteria universitaria. Il programma permette allo studente di effettuare il login, visualizzare gli appelli disponibili per il proprio corso di studi e prenotare un appello per un esame. Le funzioni incluse gestiscono la comunicazione con il server della segreteria, l'autenticazione dello studente e la presentazione di un menu per la selezione delle operazioni.

Funzionalità Specifiche

Login Studente:
La funzione Login() richiede all'utente di inserire matricola e password.
Invia le credenziali al server della segreteria per la verifica.
Riceve il risultato del login dal server e restituisce una struttura dati "Studente" in caso di successo.
Visualizzazione Appelli:
La funzione Richiesta_Visualizzazione_Appelli() invia al server una richiesta per ottenere l'elenco degli appelli disponibili per il corso di studi dello studente.
Riceve la risposta dal server e stampa a video i dettagli di ogni appello.
Prenotazione Appello:
La funzione Richiesta_Prenotazione_Appello() richiede all'utente di inserire l'ID dell'esame e dell'appello a cui desidera prenotarsi.
Invia la richiesta di prenotazione al server della segreteria.
Riceve la risposta dal server e informa l'utente dell'esito della prenotazione.

universita_lib.h
```c
#ifndef UNIVERSITA_LIB_H
#define UNIVERSITA_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include "../SQL/sqlite3.h"

// Definizioni
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MAIN_IP "127.0.0.1"
#define SERVER_PORT 1024
#define SEGRETERIA_PORT 1025
#define MALLOC 512

// Struttura da utilizzare per la creazione di thread della Segreteria
typedef struct {
    int counter;
    int serverfd;
    int segreteriafd;
    int studentefd;
    struct sockaddr_in server_address;
    struct sockaddr_in segreteria_address;
} Thread_Segreteria;

// Struttura per immagazzinare i dati dello studente
typedef struct {
    char matricola[10];
    char piano_di_studi[50];
} Studente;

// Funzioni di Framework.c
void Clear_Input_Buffer();
void Error(const char *msg);
char *Estrai_Token(char **dati);
sqlite3 *ConnessioneDB();
int Esegui_Query(sqlite3 *db, const char *sql, sqlite3_stmt **sql_query);

// Funzioni di Network.c
int Socket();
void Sock_Options(int socketfd);
void Bind(int socketfd, struct sockaddr_in address);
int Listen(int socketfd);
struct sockaddr_in Configura_Indirizzo(int port);
int Accept(int socketfd, struct sockaddr *address, socklen_t *adress_length);
void Close(int socketfd);
void Sendto(int socketfd, const char *messaggio, struct sockaddr_in address);
char *Receive(int socketfd, struct sockaddr_in *address);
void Connessione_Client(int *socketfd, struct sockaddr_in *server_address, int port);

// Funzioni di Server_Helper.c
void Verifica_Credenziali(sqlite3 *db, int segreteriafd, struct sockaddr_in segreteria_address, char *dati);
void Visualizza_Appelli(sqlite3 *db, int segreteriafd, struct sockaddr_in segreteria_address, char *dati);
void Aggiungi_Prenotazione(sqlite3 *db, int segreteriafd, struct sockaddr_in segreteria_address, char *dati);
void Aggiungi_Esame(sqlite3 *db, int segreteriafd, struct sockaddr_in segreteria_address, char *dati);
void Aggiungi_Appello(sqlite3 *db, int segreteriafd, struct sockaddr_in segreteria_address, char *dati);

// Funzioni di Segreteria_Helper.c
void *Thread_Gestione_Studente(void *arg);
void *Thread_Input_Segreteria(void *arg);
int Selezione_Richiesta_Segreteria();
void Richiesta_Aggiunta_Esame(const void *arg);
void Richiesta_Aggiunta_Appello(const void *arg);

// Funzioni di Studente_Helper.c
Studente* Login(int *studentefd, struct sockaddr_in *segreteria_address);
int Selezione_Richiesta_Studente();
void Richiesta_Visualizzazione_Appelli(int studentefd, struct sockaddr_in segreteria_address, char *piano_di_studi);
void Richiesta_Prenotazione_Appello(int studentefd, struct sockaddr_in segreteria_address, char *matricola);

#endif //UNIVERSITA_LIB_H

```

Il file universita_lib.h rappresenta l'header file principale del progetto universitario, definendo le interfacce per i vari moduli del sistema. Esso contiene dichiarazioni di funzioni, strutture dati, costanti e macro utilizzate in tutto il progetto, promuovendo la modularità, riutilizzabilità e manutenibilità del codice.

Analisi delle Dichiarazioni

Include

Il file include le librerie standard necessarie per le operazioni di input/output, manipolazione di stringhe, socket, thread, segnali e database.

Definizioni

Macro:

MAX: Definisce una macro per trovare il massimo tra due numeri.
MAIN_IP: Definisce l'indirizzo IP del server.
SERVER_PORT: Definisce la porta del server.
SEGRETERIA_PORT: Definisce la porta della segreteria.
MALLOC: Definisce una dimensione massima per l'allocazione di memoria.
Strutture:

Thread_Segreteria: Definisce una struttura per contenere informazioni relative ai thread della segreteria, come contatori, descrittori di socket e indirizzi.
Studente: Definisce una struttura per rappresentare i dati di uno studente, come matricola e piano di studi.
Funzioni

Le dichiarazioni di funzioni sono raggruppate per file di origine, suggerendo una buona organizzazione del codice:

Framework.c: Funzioni di utilità generiche per operazioni comuni come pulizia del buffer di input, gestione degli errori, estrazione di token e connessione al database.
Network.c: Funzioni per la gestione delle operazioni di rete, come creazione di socket, binding, ascolto, accettazione di connessioni, invio e ricezione di dati.
Server_Helper.c: Funzioni lato server per la gestione delle richieste della segreteria, come verifica credenziali, visualizzazione appelli, aggiunta di prenotazioni, esami e appelli.
Segreteria_Helper.c: Funzioni lato segreteria per la gestione dei thread, input dalla segreteria e richieste di aggiunta di esami e appelli.
Studente_Helper.c: Funzioni lato studente per il login, selezione delle operazioni, visualizzazione appelli e prenotazione appelli.

# Crediti

Progetto sviluppato da Eliodoro Mascolo [0124002547] e Francescomaria Luisi [0124002603] per l'esame di Reti di Calcolatori dell'anno accademico 2023/2024 dell'università Parthenope (NA) sostenuto con il professor Emanuel Di Nardo.
