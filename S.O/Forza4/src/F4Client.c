#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../inc/errExit.h"
#include "../inc/shared_memory.h"
#include "../inc/semaphore.h"
#include "../inc/matrix.h"

// memorizza gli id delle ipc
struct ipc_id{
    int semaphore;
    int shared_memory[2];
};

#define PERM S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

// var globali

struct ipc_id * ipc; // indirizzo alla struttura che memorizza gli id
int index_client; // indice del client

// il client ha perso troppo tempo a fare la mossa
void time_out(int sig){
    struct Request * request;
    request = get_shared_memory(ipc->shared_memory[1], 0);

    request->signal_index = index_client;

    kill(request->pid_server, SIGALRM);
    printf("\rHai perso troppo tempo a fare la mossa! Vince l'avversario.\n");

    free_shared_memory(request);
}

// il client è terminato spontaneamente
void quit(int sig){
    struct Request * request;
    request = get_shared_memory(ipc->shared_memory[1], 0);

    printf("\r");

    request->signal_index = index_client;

    kill(request->pid_server, SIGUSR2);
    free_shared_memory(request);
}

// quando il server termina i due processi
void term(int sig){
    struct Request * request;
    request = get_shared_memory(ipc->shared_memory[1], 0);

    if(request->vittoria == -1 && request->pid[1] != -1){ // se c'è un solo giocatore e termina
        printf("\n\rIl server è stato terminato.\n");
    }
    else if(request->vittoria == index_client){
        printf("\rL'avversario si è scollegato, hai vinto!\n");
    }

    free_shared_memory(request);
    free(ipc);

    exit(0);
}

// argv[1] -> nome
// argv[2] -> opzione *
int main(int argc, char const *argv[])
{
    int shmidMatrix; // id shmem matrice
    int shmidRequest; // id shmem struttura request
    int semid; // id della ipc dei semafori
    key_t key_matrix, key_request; // chiavi delle ipc shmem
    key_t key_sem; // chiave dei semafori

    char * matrix; // puntatore alla shmem della matrice
    struct Request * request; // puntatore alla shmem della struttura request

    int input = 0;
    int mossa = 0;  // Ritorno metodo valid_input

    // segnali
    signal(SIGALRM, time_out);
    signal(SIGINT, quit);
    signal(SIGTERM, term);

    if(argc < 2){
        printf("Errore: devi anche inserire un nome per il gicoatore, ad esempio:\n");
        printf("%s giocatore1\n", argv[0]);
        exit(1);
    }

    // creazione ipc memoria condivisa

    key_matrix = ftok("./", 'a');
    key_request = ftok("./", 'b');

    shmidRequest = shmget(key_request, sizeof(struct Request), PERM);
    if(shmidRequest == -1){
        printf("Il server non è stato ancora avviato\n");
        errExit("shmget request fallito");
    }
    request = get_shared_memory(shmidRequest, 0);
    
    shmidMatrix = shmget(key_matrix, sizeof(char)*request->row*request->col, PERM);
    if(shmidMatrix == -1)
        errExit("shmget matrice fallito");
    matrix = get_shared_memory(shmidMatrix, 0);

    // creazione ipc semafori
    key_sem = ftok("./", 'c');
    
    semid = semget(key_sem, 7, PERM);
    if(semid < 0){
        printf("Errore semafori\n");
        free_shared_memory(request);
        free_shared_memory(matrix);
        exit(1);
    }

    ipc = (struct ipc_id *) malloc(sizeof(struct ipc_id));

    ipc->shared_memory[0] = shmidMatrix;
    ipc->shared_memory[1] = shmidRequest;

    // inizializzazione dei pid e degli indici
    semOp(semid, MUTEX, -1);
    index_client = request->i;

    // se arriva un terzo client lo facciamo terminare
    if(index_client >= 2){
        printf("C'è già una partita in corso!\n");
        semOp(semid, MUTEX, 1);

        free_shared_memory(request);
        free_shared_memory(matrix);
        free(ipc);

        exit(0);
    }

    // se il secondo giocatore vuole fare una partita casuale
    // ma c'è già un giocatore che ha richiesto di fare una
    // partita normale, facciamo terminare il secondo giocatore

    if(index_client >= 1 && argc >= 3 && strcmp(argv[2], "*") == 0){
        printf("C'è già un giocatore che ha richiesto una partita normale!\n");
        semOp(semid, MUTEX, 1);
        
        free_shared_memory(request);
        free_shared_memory(matrix);
        free(ipc);

        exit(0);
    }

    request->pid[index_client] = getpid();
    request->i ++;
    semOp(semid, MUTEX, 1);

    semOp(semid, C, 1); // sblocco server

    printf("Gioco Forza4\tGiocatore: %s (%c)\n", argv[1], request->gettone);
    stampaMatrice(matrix, request->row, request->col);

    // partita casuale
    if(argc >= 3 && strcmp(argv[2], "*") == 0){
        kill(request->pid_server, SIGUSR1);
    }

    // attendiamo che arrivi l'altro giocatore prima di iniziare
    semOp(semid, PLAYER, -1);

    if(index_client == 0)
        printf("Attendi l'arrivo dell'altro giocatore...\n");

    semOp(semid, PLAYER, 0);

    semOp(semid, MUTEX, -1); // mutex vittoria
    while(request->vittoria == -1){
        semOp(semid, MUTEX, 1); // mutex vittoria

        if(request->turno != -1){ // stampa solo quando almeno uno ha fatto una giocata
            write(1, "Tocca all'avversario...\n", 24); // faccio con write a causa del buffer, 1 indica lo stdout
        }

        semOp(semid, index_client, -1); // P(sem[index_client])

        if(request->turno != -1) { // se turno = -1 vuol dire che il primo giocatore non ha ancora fatto la sua giocata
            printf("L'avversario ha giocato:\n");
            stampaMatrice(matrix, request->row, request->col);
        }

        if(request->vittoria == -1) {
            semOp(semid, IN2, -1);

            // è il mio turno
            semOp(semid, MUTEX, -1);
            request->turno = index_client;
            semOp(semid, MUTEX, 1);

            semOp(semid, TURNO, 1);

            printf("<%s> E' il tuo turno\n", argv[1]);
            do{
                printf("<%s> Scegli in quale colonna giocare: ", argv[1]);

                alarm(request->time_out); // imposto l'allarme
                scanf("%d", &input);
                alarm(0); // disattivo l'allarme

                mossa = valid_input(matrix, input, request->col);
                if(!mossa && (input < 1 || input > request->col))
                    printf("<%s> La colonna non è all'interno della matrice!\n", argv[1]);
                else if(!mossa)
                    printf("<%s> Hai inserito il gettone in una colonna piena!\n", argv[1]);
            }while(!mossa);
            request->input = input;

            semOp(semid, IN1, 1);

            semOp(semid, VITTORIA, -1);

            semOp(semid, STAMPA, -1);

            stampaMatrice(matrix, request->row, request->col);
        }
        
        semOp(semid, (index_client == 1) ? 0 : 1, 1);

        semOp(semid, MUTEX, -1); // mutex vittoria
    }
    semOp(semid, MUTEX, 1); // mutex vittoria

    if(index_client == request->vittoria)
        printf("<%s> Hai vinto\n", argv[1]);
    else if(request->vittoria != 2)    // se non è pareggio
        printf("<%s> Hai perso\n", argv[1]);
    else
        printf("<%s> Pareggio\n", argv[1]);

    free_shared_memory(request);
    free_shared_memory(matrix);
    free(ipc);

    semOp(semid, TERMINE, -1);

    return 0;
}
