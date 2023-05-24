#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../inc/errExit.h"
#include "../inc/shared_memory.h"

/*int alloc_shared_memory(key_t key, size_t size){
    int shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmid == -1)
        errExit("shmget fallito");
    return shmid;
}*/

void *get_shared_memory(int shmid, int shmflg){
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if(ptr_sh == (void * )-1)
        errExit("shmat fallito");
    return ptr_sh;
}


void free_shared_memory(void *ptr_sh){
    if(shmdt(ptr_sh)==-1)
        errExit("hmdt falito");
}

void remove_shared_memory(int shmid){
    if(shmctl(shmid, IPC_RMID,NULL)==-1)
        errExit("shmctl fallito");
}



