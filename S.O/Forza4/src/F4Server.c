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

int create_sem_set(key_t semkey) {
    // Create a semaphore set with 2 semaphores
    int semid = semget(semkey, 3, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("semget failed");

    // Initialize the semaphore set
    union semun arg;
    unsigned short values[] = {0, 1};
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1)
        errExit("semctl SETALL failed");

    return semid;
}

int InserisciGettone(int colonna, char *M, int row, int col, char gettone){ //Giocatore, colonna inserimento, matrice 
    int i,j, ret = 0;
    
    for(i = row-1; i >= 0 && ret == 0; i--) //Scorro la matrice dal basso
        for(j = col-1; j >= 0 && ret == 0; j--)
            if(j == colonna-1 && *(M+(i*col)+j) != 'X' && *(M+(i*col)+j) != 'O'){ //Mi porto alla prima casella libera della colonna richiesta
                *(M+(i*col)+j) = gettone;
                ret = 1;    //Gettone inserito
            }
    
    return ret; //Ritorna 0 solo in caso di colonna piena
}

int main (int argc, char *argv[]) {
    if (argc != 5){
        printf("<Server> Devi inserire %s nRighe nColonne e i 2 gettoni->(O e X)\n", argv[0]);
        exit(1);
    }
    //Lettura dimensione matrice
    int row = atoi(argv[1]);
    if(row <= 4){
        printf("<Server> Le righe della matrice devono essere almeno di 5\n");
        exit(1);
    }
    int col = atoi(argv[2]);
    if(col <= 4){
        printf("<Server> Le Colonne della matrice devono essere almeno di 5\n");
        exit(1);
    }
    //Lettura gettoni
    char G1 = 'O';
    char G2 = 'X';
    char gettone1 = *argv[3]; 

    if (gettone1 != G1 && gettone1 != G2){
        printf("<Server> argv[3] devi mettere i getoni O o X\n");
        exit(1);
    }

    char gettone2 = *argv[4];
    if (gettone2 != G1 && gettone2 != G2 && gettone2 == gettone1){
        printf("<Server> argv[4] devi mettere i getoni O o X\n");
        exit(1);
    }


    key_t key_matrix, key_request;
    printf("<Server> Creo la memoria condivisa per la matrice...\n");
    key_matrix = ftok("./", 'a');
    key_request = ftok("./", 'b');
    int shmidServer = shmget(key_matrix, sizeof(char)*row*col, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmidServer == -1)
        errExit("shmget matrice fallito");
    int shmidRequest = shmget(key_request, sizeof(struct Request), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmidRequest == -1)
        errExit("shmget request fallito");
    printf("<Server> Collego la memoria condivisa per la matrice...\n");
    char *matrix = get_shared_memory(shmidServer, 0);
    struct Request *request = get_shared_memory(shmidRequest, 0); 
    request->row = row;
    request->col = col;
    printf("<Server> Matrice pronta... (%d %d)\n", request->col, request->row);

    int i, j;
    for(i = 0; i < row; i++)    //Inizializzo la matrice
        for(j = 0; j < col; j++)
            *(matrix+(i*col)+j) = '/';

    //Inizializzo semafori
    key_t key_sem = ftok("./", 'c');
    int pid1, pid2;
    int semid = create_sem_set(key_sem);
    request->gettone = gettone1;
    printf("<Server> In attesa dei client\n");
    semOp(semid, REQUEST, -1);  //Blocco il server in attesa del client 1
    pid1 = request->pid;
    printf("<Server> Player 1 collegato\n");
    request->gettone = gettone2;
    semOp(semid, REQUEST, -1);  //Blocco il server in attesa del client 2
    pid2 = request->pid;
    printf("<Server> Player 2 collegato\n<Server> In attesa di giocata...\n");
    semOp(semid, REQUEST, -1);  //Blocco il server in attesa di giocata
    InserisciGettone(request->input, matrix, row, col, request->gettone);
    semOp(semid, PLAYER2, 1);

    free_shared_memory(request);
    free_shared_memory(matrix);
    remove_shared_memory(shmidServer);
    remove_shared_memory(shmidRequest);
    
    return 0;
}
