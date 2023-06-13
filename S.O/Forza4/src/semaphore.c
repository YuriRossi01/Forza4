#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "../inc/semaphore.h"
#include "../inc/errExit.h"

// retry: ritenta il blocco, è utile nello specifico caso in cui
// arriva il primo segnale ctrlc, infatti in tal caso non bisogna
// terminare il processo ma bisogna continuare avanti.

// retry = 0 -> non ritenta il blocco sul semaforo
// retry = 1 -> ritenta il blocco sul semaforo

void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};
    int res;
    int retry = 1;

    do {
        res = semop(semid, &sop, 1);
        
        if(!retry && res == -1)
            errExit("semop failed\n");

    } while(retry && res == -1);
}

void remove_semaphore(int semid){
    if(semctl(semid, 0, IPC_RMID, 0) == -1)
        errExit("semctl fallito");
}
