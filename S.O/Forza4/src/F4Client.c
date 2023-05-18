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

#define KEYSHM 15
#define KEYSEM 16
#define REQUEST 0
#define PLAYER1 1
#define PLAYER2 1

int main(int argc, char const *argv[])
{

    if(argc != 2){
        printf("Errore input\n");
    }
    //Semaforo del server
    int semid = semget(KEYSEM, 2, S_IRUSR | S_IWUSR);
    if (semid > 0) {
        //Sblocco il server 
        semOp(semid, REQUEST, 1);
        printf("%s ", argv[1]);
        //Ottengo la shm del server
        int shmidServer = alloc_shared_memory(KEYSHM, sizeof(struct Request));
        //Il client accede alla memoria condivisa del server
        struct Request *request = (struct Request *)get_shared_memory(shmidServer, 0);
        int i,j;
        printf("%d %d ", request->col, request->row);
        // wait for data
        //semOp(semid, PLAYER1, -1);
    } else
        printf("semget failed");
    return 0;
}