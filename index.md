---
title: Università
author: Eliodoro Mascolo
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
Server universitario:

Riceve l'aggiunta di nuovi esami
Riceve la prenotazione di un esame


Il server universitario ad ogni richiesta di prenotazione invia alla segreteria il numero di prenotazione progressivo assegnato allo studente e la segreteria a sua volta lo inoltra allo studente 

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

Network.c fornisce un set di funzioni per la gestione di socket TCP IPv4. Le funzionalità implementate includono la creazione, la configurazione, il binding, l'ascolto, l'accettazione di connessioni, l'invio e la ricezione di messaggi, la chiusura di socket e la connessione a un server remoto. Il codice presenta una struttura modulare ben organizzata e un uso appropriato dei commenti per migliorare la leggibilità.
Funzionalità Principali
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

Server_Helper implementa diverse funzionalità per la gestione delle comunicazioni tra un server e un'applicazione client, in particolare con focus sulla gestione di richieste da parte degli studenti e l'esecuzione di operazioni su un database. Le funzioni incluse gestiscono la ricezione di dati dal client, la verifica delle credenziali degli utenti, l'esecuzione di query SQL su un database, la formattazione e l'invio di risposte al client.

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

# Crediti

Progetto sviluppato da Eliodoro Mascolo [0124002547] per l'esame di Reti di Calcolatori dell'anno accademico 2023/2024 dell'università Parthenope (NA) sostenuto con il professor Emanuel Di Nardo.
