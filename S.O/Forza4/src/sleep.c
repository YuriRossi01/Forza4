#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "errExit.h"

//gestore del segnale SIGALRM
void sigHandler(int sig){
    printf("Giocatore Trovato...");
}

int main(int argc, char const *argv[])
{
    if (argc != 2){
        printf("usare : %s seconds", argv[0]);
        return 1;
    }
    
    int sleepFor = atoi(argv[1]);
    if (sleepFor <=0)
        return 1;
    // se il SIGALRT è ugiale al SIG_ERR -> errore
    if(signal(SIGABRT, sigHandler)== SIG_ERR)
        errExit("gestiore signele di modifica è fallito ");

    // timer in secondi
    alarm(sleepFor);
    //pausa 
    pause();
    
    return 0;
}

