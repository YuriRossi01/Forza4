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

#define REQUEST 0
#define PLAYER 1
#define PLAYER2 0

void stampaMatrice(char *matrix, int row, int col){
    int i,j;
    for(i = 0; i < row; i++){    //Stampo la matrice
            for(j = 0; j < col; j++)
                printf("%c ", *(matrix+(i*col)+j));
            printf("\n");
        }
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
    //Semaforo del server
    key_t key_sem = ftok("./", 'c');
    int semid = semget(key_sem, 3, S_IRUSR | S_IWUSR);
    if (semid >= 0) {
        request->pid = getpid();
        //Sblocco il server 
        semOp(semid, REQUEST, 1);
        printf("Gioco Forza4\tGiocatore: %s(%c)\nDimensione campo da gioco: (%d %d)\n", argv[1], request->gettone, request->col, request->row);
        int input;
        stampaMatrice(matrix, request->row, request->col);
        // wait for data
        printf("<Client> Attesa turno di gioco...\n");
        semOp(semid, PLAYER, -1);
        printf("<Client> E' il tuo turno\nScegli in quale colonna giocare: ");
        scanf("%d", &input);
        request->input = input;
        semOp(semid, REQUEST, 1);
        semOp(semid, PLAYER2, -1);
        stampaMatrice(matrix, request->row, request->col);
    } else
        printf("semget failed\n");
    return 0;
}