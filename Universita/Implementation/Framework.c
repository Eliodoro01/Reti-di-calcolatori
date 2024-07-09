// Framework.c
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
