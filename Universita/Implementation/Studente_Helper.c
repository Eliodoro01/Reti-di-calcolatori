// Studente_Helper.c
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
        printf("\nLa prenotazione è andata a buon fine: ID Prenotazione = %s.\n", id_prenotazione);
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
