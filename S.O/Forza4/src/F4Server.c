#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "LibrerieInterne/errExit.h"
#include "LibrerieInterne/shared_memory.h"
#define G1 O
#define G2 X

unsigned int sizeof_dm(int rows, int cols, size_t sizeElement){
    size_t size = rows * (sizeof(void *) + (cols * sizeElement));
    return size;
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
    size_t sizeMatrix = sizeof_dm(row,col,sizeof(int));
    // allocate a shared memory segment
    // ho creato una memoria con  matrice, righe e collonne
    int shmidServer = alloc_shared_memory(IPC_PRIVATE,sizeMatrix);
    // attacco alla mem cond le row e le col
    struct Request *request = malloc(sizeof(struct Request));
    request->shmid = (struct Request*)get_shared_memory(shmidServer,0);
    //struct Request *request = (struct Request*)get_shared_memory(shmidServer,0);
    request->row = row;
    request->col = col;
    printf("ho scritto righe e col ");
    
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
