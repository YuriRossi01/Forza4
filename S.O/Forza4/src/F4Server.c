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

#define G1 O
#define G2 X

#define REQUEST 0
#define PLAYER1 1
#define PLAYER2 1

int create_sem_set(key_t semkey) {
    // Create a semaphore set with 2 semaphores
    int semid = semget(semkey, 3, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("semget failed");

    // Initialize the semaphore set
    union semun arg;
    unsigned short values[] = {0, 0, 0};
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1)
        errExit("semctl SETALL failed");

    return semid;
}

int main (int argc, char *argv[]) {
    if (argc != 3){
        printf("devi inserire %s nRighe nColonne e i 2 gettoni->(O e X)\n", argv[0]);
        exit(1);
    }
    int row = atoi(argv[1]);
    if(row <= 4){
        printf("le righe della matrice devono essere almeno di 5\n");
        exit(1);
    }
    int col = atoi(argv[2]);
    if(col <= 4){
        printf("le Colonne della matrice devono essere almeno di 5\n");
        exit(1);
    }
    key_t key_matrix, key_request;
    printf("<Server> Creo la memoria condivisa per la matrice...\n");
    key_matrix = ftok("./", 'a');
    key_request = ftok("./", 'b');
    int shmidServer = shmget(key_matrix, sizeof(int)*row*col, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmidServer == -1)
        errExit("shmget matrice fallito");
    int shmidRequest = shmget(key_request, sizeof(struct Request), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmidRequest == -1)
        errExit("shmget request fallito");
    printf("<Server> Collego la memoria condivisa per la matrice...\n");
    int *matrix = get_shared_memory(shmidServer, 0);
    struct Request *request = get_shared_memory(shmidRequest, 0); 
    request->row = row;
    request->col = col;
    printf("<Server> Matrice pronta...\n%d %d", request->col, request->row);

    //Inizializzo semafori
    key_t key_sem = ftok("./", 'c');
    int semid = create_sem_set(key_sem);
    printf("<Server> In attesa dei client\n");
    semOp(semid, REQUEST, -1);  //Blocco il server in attesa del client 1
    printf("<Server> Player 1 collegato\n");
    semOp(semid, REQUEST, -1);  //Blocco il server in attesa del client 2
    printf("<Server> Player 2 collegato\n");
    //printf("%d %d", request->pid1, request->pid2);
    //semOp(semid, REQUEST, 1);     //Sblocca il server
    //free_shared_memory(request);
    //remove_shared_memory(shmidServer);






    /*
    char gettone1 = argv[3]; 
    
    if (gettone1 != G1 && gettone1 != G2){
        printf("argv[3] devi mettere i getoni O o X\n");
        exit(1);
    }
    else
        printf("bravo argc[3]");

    char gettone2 =argv[4];
    if (gettone2 != G1 && gettone2 != G2 && gettone2 == gettone1){
        printf("argv[4] devi mettere i getoni O o X\n");
        exit(1);
    }
    else
        printf("bravo argc[4]");
    */
// dobbiamo  mettere nella mem condivisa le seguenti cose:
/*  matrice composta da...
    row:  date da input da CMD
    col: date da input da CMD
    gettoni: date da input da CMD
*/
// salvo nella mem condivisa la row:
    //int shmidRow = alloc_shared_memory(request->row,sizeof(struct Request));
    
    return 0;
}
