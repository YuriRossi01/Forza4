#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

//definition of the union semun
union semaphore
{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};
void semOp(int semid, unsigned short sem_num, short sem_op);
#endif