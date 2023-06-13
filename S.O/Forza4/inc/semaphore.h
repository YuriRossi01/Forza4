#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

// Semafori utilizzati
#define SEM1 0
#define SEM2 1
#define C 2
#define IN1 3
#define IN2 4
#define MUTEX 5
#define STAMPA 6
#define TURNO 7
#define PLAYER 8 // attesa che l'altro giocatore entri in campo

//definition of the union semun
union semun
{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void semOp(int semid, unsigned short sem_num, short sem_op);
void remove_semaphore(int semid);
#endif