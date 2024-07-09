// Server_Helper
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
