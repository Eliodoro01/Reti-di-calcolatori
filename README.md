# Università

### Progetto Universitario di Reti di Calcolatori 2023/2024

Studente: Francesco Maria Luisi

Professore: Emanuel Di Nardo

## Traccia:

### Scrivere un'applicazione client/server parallelo per gestire esami universitari.

#### Segreteria:

- Inserisce gli esami sul server dell'università (salvare in un file o conservare in memoria il dato);
- Inoltra la richiesta di prenotazione degli studenti al server universitario;
- Fornisce allo studente le date degli esami disponibili per l'esame scelto dallo studente.

#### Studente:

- Chiede alla segreteria se ci siano esami disponibili per un corso;
- Invia una richiesta di prenotazione di un esame alla segreteria.

#### Server Universitario:

- Riceve l'aggiunta di nuovi esami;
- Riceve la prenotazione di un esame.

### Condizioni Aggiuntive: Gruppo 2 Studenti

Il server universitario ad ogni richiesta di prenotazione invia alla segreteria il numero di prenotazione progressivo
assegnato allo studente e la segreteria a sua volta lo inoltra allo studente.