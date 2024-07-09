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
