#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "../inc/semaphore.h"
#include "../inc/errExit.h"

void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};
    int res;

    do {
        res = semop(semid, &sop, 1);
    } while(res == -1);

}

void remove_semaphore(int semid){
    if(semctl(semid, 0, IPC_RMID, 0) == -1)
        errExit("semctl fallito");
}
