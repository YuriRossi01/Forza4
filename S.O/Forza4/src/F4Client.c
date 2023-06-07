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

#include "../inc/errExit.h"
#include "../inc/shared_memory.h"
#include "../inc/semaphore.h"

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

int main(int argc, char const *argv[])
{

    if(argc != 2){
        printf("Errore input\n");
        exit(1);
    }

    //Prendo la memoria condivisa per la matrice
    key_t key_matrix = ftok("./", 'a');
    key_t key_request = ftok("./", 'b');
    int shmidRequest = shmget(key_request, sizeof(struct Request), IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmidRequest == -1)
        errExit("shmget request fallito");
    struct Request *request = get_shared_memory(shmidRequest, 0); 
    int shmidServer = shmget(key_matrix, sizeof(char)*request->row*request->col, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmidServer == -1)
        errExit("shmget matrice fallito");
    char *matrix = get_shared_memory(shmidServer, 0);
    //Collego la memoria condivisa per la matrice

    key_t key_sem = ftok("./", 'c');
    int semid = semget(key_sem, 7, S_IRUSR | S_IWUSR);
    int input = 0;
    char in;
    int index; // indice del processo

    if(semid < 0){
        printf("Errore semafori\n");
        exit(1);
    }

    // controllare poi che il server arrivi prima dei client
    // controllare che il primo client attendi il secondo

    // inizializzazione dei pid e degli indici
    semOp(semid, MUTEX, -1);
    index = request->i;
    request->pid[index] = getpid();
    request->i ++;
    semOp(semid, MUTEX, 1);

    semOp(semid, C, 1); // sblocco server
    
    stampaMatrice(matrix, request->row, request->col);

    while(1){
        
        if(request->turno != -1){ // stampa solo quando almeno uno ha fatto una giocata
            write(1, "In attesa dell'altro giocatore...\n", 34); // faccio con write a causa del buffer, 1 indica lo stdout
        }

        semOp(semid, index, -1); // P(sem[index])

        if(request->turno != -1){ // se turno = -1 vuol dire che il primo giocatore non ha ancora fatto la sua giocata
            printf("L'altro giocatore ha giocato:\n");
            stampaMatrice(matrix, request->row, request->col);
        }

        // è il mio turno
        semOp(semid, MUTEX, -1);
        request->turno = index;
        semOp(semid, MUTEX, 1);

        semOp(semid, IN2, -1);

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

        semOp(semid, (index == 1) ? 0 : 1, 1);
    }

    return 0;
}