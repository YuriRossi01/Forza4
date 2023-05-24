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
#define PLAYER1 1
#define PLAYER2 1

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
    int shmidServer = shmget(key_matrix, sizeof(int)*request->row*request->col, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmidServer == -1)
        errExit("shmget matrice fallito");
    int *matrix = get_shared_memory(shmidServer, 0);
    //Collego la memoria condivisa per la matrice
    //Semaforo del server
    key_t key_sem = ftok("./", 'c');
    int semid = semget(key_sem, 2, S_IRUSR | S_IWUSR);
    if (semid >= 0) {
        //Sblocco il server 
        semOp(semid, REQUEST, 1);
        printf("Gioco Forza4\tGiocatore: %s\n%d %d", argv[1], request->col, request->row);
        //request->pid1 = getpid();
        //request->pid2 = getpid();
        int i,j;
        // wait for data
        //semOp(semid, PLAYER1, -1);
    } else
        printf("semget failed\n");
    return 0;
}