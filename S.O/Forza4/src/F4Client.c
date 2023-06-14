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

//#define SIGUSR 10

// memorizza gli id delle ipc
struct ipc_id{
    int semaphore;
    int shared_memory[2];
};

// var globali
// indirizzo alla struttura che memorizza gli id
struct ipc_id * ipc;

void stampaMatrice(char * matrix, int row, int col){
    int i, j;
    printf("-------------------------------------------\n");
    for(i = 0; i < row; i ++){
        for(j = 0; j < col; j++)
            printf("%c ", *(matrix+(i*col)+j));
        printf("\n");
    }
    printf("-------------------------------------------\n");
}

// parametri:
// matrice, input, colonne max della matrice
int valid_input(char * matrix, int input, int col){
    return (input <= col) && (input >= 1) && (*(matrix + input - 1) == '/'); // matrix[0][input - 1]
}

// argv[1] -> nome
// argv[2] -> opzione *
int main(int argc, char const *argv[])
{

    if(argc < 2){
        printf("Errore input\n");
        exit(1);
    }

    ipc = (struct ipc_id *) malloc(sizeof(struct ipc_id));

    //Prendo la memoria condivisa per la matrice
    key_t key_matrix = ftok("./", 'a');
    key_t key_request = ftok("./", 'b');
    int shmidRequest = shmget(key_request, sizeof(struct Request), IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmidRequest == -1)
        errExit("shmget request fallito");
    struct Request * request = get_shared_memory(shmidRequest, 0); 
    int shmidServer = shmget(key_matrix, sizeof(char)*request->row*request->col, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmidServer == -1)
        errExit("shmget matrice fallito");
    char *matrix = get_shared_memory(shmidServer, 0);
    //Collego la memoria condivisa per la matrice

    key_t key_sem = ftok("./", 'c');
    int semid = semget(key_sem, 7, S_IRUSR | S_IWUSR);
    int input = 0;
    char in;
    int index_client;

    if(semid < 0){
        printf("Errore semafori\n");
        exit(1);
    }

    ipc->shared_memory[0] = shmidServer;
    ipc->shared_memory[1] = shmidRequest;

    // inizializzazione dei pid e degli indici
    semOp(semid, MUTEX, -1);
    index_client = request->i;

    // se arriva un terzo client lo facciamo terminare
    if(index_client >= 2){
        printf("C'è già una partita in corso!\n");
        exit(0);
    }

    request->pid[index_client] = getpid();
    request->i ++;
    semOp(semid, MUTEX, 1);

    semOp(semid, C, 1); // sblocco server

    stampaMatrice(matrix, request->row, request->col);

    // partita casuale
    if(argc >= 3 && strcmp(argv[2], "*") == 0){
        kill(request->pid_server, SIGUSR1);
    }

    // attendiamo che arrivi l'altro giocatore prima di iniziare
    semOp(semid, PLAYER, -1);

    semOp(semid, MUTEX, -1);
    if(index_client == 0)
        printf("Attendi l'arrivo dell'altro giocatore...\n");
    semOp(semid, MUTEX, 1);

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

            printf("<Client> E' il tuo turno\n");
            do{
                printf("<Client> Scegli in quale colonna giocare: ");
                scanf("%d", &input);
                if(!valid_input(matrix, input, request->col))
                    printf("<Client> La colonna non è all'interno della matrice oppure hai inserito il gettone in una colonna piena!\n");
            }while(!valid_input(matrix, input, request->col));
            request->input = input;

            semOp(semid, IN1, 1);

            semOp(semid, STAMPA, -1);

            stampaMatrice(matrix, request->row, request->col);
        }
        
        semOp(semid, (index_client == 1) ? 0 : 1, 1);

        semOp(semid, MUTEX, -1); // mutex vittoria
    }
    semOp(semid, MUTEX, 1); // mutex vittoria

    printf("Vincitore: giocatore %i\n", request->vittoria + 1);

    semOp(semid, REQUEST, -1);

    return 0;
}