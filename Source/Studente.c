// Studente.c
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
