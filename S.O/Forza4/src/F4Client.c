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
#define PLAYER2 2
#define MATRIX 3
#define MATRIX2 4

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
    int semid = semget(key_sem, 5, S_IRUSR | S_IWUSR);
    if (semid >= 0) {
        request->pid = getpid();
        //Sblocco il server 
        semOp(semid, REQUEST, 1);
        printf("Gioco Forza4\tGiocatore: %s(%c)\nDimensione campo da gioco: (%d %d)\n", argv[1], request->gettone, request->col, request->row);
        int input;
        while(1){
            semOp(semid, MATRIX, -1);   //Il client chiede accesso alla matrice
            stampaMatrice(matrix, request->row, request->col);
            printf("<Client> Attesa turno di gioco...\n");
            semOp(semid, PLAYER, -1);   //Il primo client passa, il secondo si blocca
            semOp(semid, PLAYER2, -1);  //Il client si blocca in attesa di autorizzazione dal server
            printf("<Client> E' il tuo turno\n");
            do{
                printf("<Client> Scegli in quale colonna giocare: ");
                scanf("%d", &input);
                if(input < 1 || input > request->col)
                    printf("<Client> La colonna non è all'interno della matrice\n");
            }while(input < 1 || input > request->col);
            request->input = input;
            semOp(semid, REQUEST, 1);   //Sblocca il server che inserisce il gettone nella colonna richiesta
            semOp(semid, MATRIX2, -1);   //Si blocca in attesa di permesso dal server per accedere alla matrice aggiornata
            stampaMatrice(matrix, request->row, request->col);
            printf("<Client> Turno terminato...\n");
        }
    } else
        printf("semget failed\n");
    return 0;
}