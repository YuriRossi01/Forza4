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
#include <time.h>

#include "../inc/errExit.h"
#include "../inc/shared_memory.h"
#include "../inc/semaphore.h"
#include "../inc/matrix.h"

// memorizza gli id delle ipc
struct ipc_id{
    int semaphore;
    int shared_memory[2];
};

// var globali
// indirizzo alla struttura che memorizza gli id
struct ipc_id * ipc;

void remove_ipc(){
    remove_shared_memory(ipc->shared_memory[0]);
    remove_shared_memory(ipc->shared_memory[1]);
    remove_semaphore(ipc->semaphore);
    free(ipc);
}

int create_sem_set(key_t semkey) {
    // Create a semaphore set with 2 semaphores
    int semid = semget(semkey, 11, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("semget failed");

    // Initialize the semaphore set
    union semun arg;
    unsigned short values[] = {1, 0, 0, 0, 1, 1, 0, 0, 2, 2, 0};
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1)
        errExit("semctl SETALL failed");

    return semid;
}

void second_ctrlc(int sig){
    struct Request *request = get_shared_memory(ipc->shared_memory[1], 0);

    printf("\rGrazie per aver giocato!\n");
    
    remove_ipc();
    if(request->pid[0] != -1)
        kill(request->pid[0], SIGTERM);
    if(request->pid[1] != -1)
        kill(request->pid[1], SIGTERM);
    
    exit(0);
}

void first_ctrlc(int sig){
    printf("\r<Server> Premere nuovamente Ctrl-c per terminare l'esecuzione\n");
    signal(2, second_ctrlc);
}

// un client si è scollegato
void term_client(int sig){
    struct Request *request = get_shared_memory(ipc->shared_memory[1], 0);

    printf("<Server> Player %i si è scollegato\n", request->signal_index + 1);
    request->vittoria = (request->signal_index == 0) ? 1 : 0;
    

    if(request->pid[0] != -1)
        kill(request->pid[0], SIGTERM);
    if(request->pid[1] != -1)
        kill(request->pid[1], SIGTERM);

    free_shared_memory(request);
    remove_ipc();

    exit(0);
}

void casual_game(){
    int semid = ipc->semaphore;
    int shmidMatrix = ipc->shared_memory[0];
    int shmidRequest = ipc->shared_memory[1];
    char * matrix = get_shared_memory(shmidMatrix, 0);
    struct Request *request = get_shared_memory(shmidRequest, 0);
    int index_client;
    int input;

    srand(time(NULL));

    // inizializzazione dei pid e degli indici
    semOp(semid, MUTEX, -1);
    index_client = request->i;
    request->pid[index_client] = getpid();
    request->i ++;
    semOp(semid, MUTEX, 1);

    semOp(semid, C, 1); // sblocco server

    stampaMatrice(matrix, request->row, request->col);

    // sblocco dell'altro giocatore che aspetta che player arrivi a 0
    semOp(semid, PLAYER, -1);

    semOp(semid, MUTEX, -1); // mutex vittoria
    while(request->vittoria == -1){
        semOp(semid, MUTEX, 1); // mutex vittoria

        if(request->turno != -1){ // stampa solo quando almeno uno ha fatto una giocata
            write(1, "Tocca all'avversario...\n", 24); // faccio con write a causa del buffer, 1 indica lo stdout
        }

        semOp(semid, index_client, -1); // P(sem[index_client])

        if(request->turno != -1) { // se turno = -1 vuol dire che il primo giocatore non ha ancora fatto la sua giocata
            printf("L'avversario ha giocato:\n");
            stampaMatrice(matrix, request->row, request->col);
        }

        if(request->vittoria == -1) {
            semOp(semid, IN2, -1);

            // è il mio turno
            semOp(semid, MUTEX, -1);
            request->turno = index_client;
            semOp(semid, MUTEX, 1);

            semOp(semid, TURNO, 1);

            printf("<Server> Selezione input\n");
            do{
                input = rand() % request->col + 1;
                if(!valid_input(matrix, input, request->col))
                    printf("Colonna piena, ritento\n");
            }while(!valid_input(matrix, input, request->col));
            request->input = input;

            semOp(semid, IN1, 1);

            semOp(semid, STAMPA, -1);

            stampaMatrice(matrix, request->row, request->col);
        }
        
        semOp(semid, (index_client == 1) ? 0 : 1, 1);

        semOp(semid, MUTEX, -1); // mutex vittoria
    }
    semOp(semid, MUTEX, 1); // mutex vittoria

    if(index_client == request->vittoria)
        printf("Hai vinto\n");
    else if(request->vittoria != 2)
        printf("Hai perso\n");
    else
        printf("Pareggio\n");

    free_shared_memory(request);
    free_shared_memory(matrix);

    semOp(semid, TERMINE, -1);
}

void casual_game_handler(int sig){
    printf("Inizio gioco casuale\n");

    int pid = fork();
    if(pid == 0){

        // reimposto i segnali di default
        signal(SIGINT, SIG_DFL);
        signal(SIGUSR1, SIG_DFL);
        signal(SIGUSR2, SIG_DFL);
        signal(SIGALRM, SIG_DFL);
        
        casual_game();
        exit(0);
    }
}

void time_out(int sig){
    struct Request * request;
    request = get_shared_memory(ipc->shared_memory[1], 0);

    printf("Il Giocatore ha impiegato troppo tempo a fare la mossa.\n");

    request->vittoria = (request->signal_index == 0) ? 1 : 0;

    if(request->pid[0] != -1)
        kill(request->pid[0], SIGTERM);
    if(request->pid[1] != -1)
        kill(request->pid[1], SIGTERM);

    free_shared_memory(request);
    remove_ipc();

    exit(0);
}

int main (int argc, char *argv[]) {
    int col;
    int row;
    char G1 = 'O';
    char G2 = 'X';
    char gettone1;
    char gettone2;
    int shmidMatrix; // id shmem matrice
    int shmidRequest; // id shmem struttura request
    int semid; // id della ipc dei semafori
    key_t key_matrix, key_request; // chiavi delle ipc shmem
    key_t key_sem; // chiave dei semafori

    char * matrix; // puntatore alla shmem della matrice
    struct Request * request; // puntatore alla shmem della struttura request

    ipc = (struct ipc_id *) malloc(sizeof(struct ipc_id));

    // segnali
    signal(2, first_ctrlc);
    signal(SIGUSR1, casual_game_handler);
    signal(SIGALRM, time_out);
    signal(SIGUSR2, term_client);

    // controllo parametri
    if (argc != 5){
        printf("<Server> Devi inserire %s nRighe nColonne e i 2 gettoni->(O e X)\n", argv[0]);
        exit(1);
    }

    // Lettura dimensione matrice
    if(row <= 4){
        printf("<Server> Le righe della matrice devono essere almeno di 5\n");
        exit(1);
    }
    if(col <= 4){
        printf("<Server> Le Colonne della matrice devono essere almeno di 5\n");
        exit(1);
    }

    // Lettura gettoni
    if (gettone1 != G1 && gettone1 != G2){
        printf("<Server> argv[3] devi mettere i gettoni O o X\n");
        exit(1);
    }
    if (gettone2 != G1 && gettone2 != G2 && gettone2 == gettone1){
        printf("<Server> argv[4] devi mettere i gettoni O o X\n");
        exit(1);
    }

    // atoi ritorna 0 in caso di fallimento
    col = atoi(argv[2]);
    row = atoi(argv[1]);
    gettone1 = *argv[3];
    gettone2 = *argv[4];

    // creazione delle ipc di memoria condivisa

    printf("<Server> Creo la memoria condivisa per la matrice...\n");
    key_matrix = ftok("./", 'a');
    key_request = ftok("./", 'b');
    
    shmidMatrix = shmget(key_matrix, sizeof(char) * row * col, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmidMatrix == -1)
        errExit("shmget matrice fallito");
    
    shmidRequest = shmget(key_request, sizeof(struct Request), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shmidRequest == -1)
        errExit("shmget request fallito");

    ipc->shared_memory[0] = shmidMatrix;
    ipc->shared_memory[1] = shmidRequest;
    
    printf("<Server> Collego la memoria condivisa per la matrice...\n");
    matrix = get_shared_memory(shmidMatrix, 0);
    request = get_shared_memory(shmidRequest, 0);

    request->row = row;
    request->col = col;
    request->pid_server = getpid();
    request->vittoria = -1;
    request->pid[0] = -1;
    request->pid[1] = -1;
    request->time_out = 20;
    
    printf("<Server> Matrice pronta... (%d %d)\n", request->col, request->row);

    // Inizializzo la matrice
    int i, j;
    for(i = 0; i < row; i++)
        for(j = 0; j < col; j++)
            *(matrix+(i*col)+j) = '/';

    // Inizializzazione semafori
    key_sem = ftok("./", 'c');
    semid = create_sem_set(key_sem);

    ipc->semaphore = semid;

    printf("<Server> In attesa dei client\n");

    request->i = 0;
    request->turno = -1;

    request->gettone = gettone1;
    semOp(semid, C, -1);
    printf("<Server> Player 1 collegato\n");

    request->gettone = gettone2;
    semOp(semid, C, -1);
    printf("<Server> Player 2 collegato\n");

    semOp(semid, MUTEX, -1);
    while(request->vittoria == -1){
        semOp(semid, MUTEX, 1);

        semOp(semid, TURNO, -1);
        printf("<Server> In attesa di giocata di player %i\n", request->turno + 1);
        
        semOp(semid, IN1, -1); // P(IN1) blocco in attesa dell'input del client

        // controllo di chi è il turno per scegliere il gettone
        semOp(semid, MUTEX, -1); // mutex per accesso alla var condivisa turno
        if(request->turno == 0) {
            semOp(semid, MUTEX, 1);
            request->gettone = gettone1;
        }
        else {
            semOp(semid, MUTEX, 1);
            request->gettone = gettone2;
        }

        InserisciGettone(request->input, matrix, row, col, request->gettone);
        semOp(semid, STAMPA, 1);

        semOp(semid, IN2, 1);

        if(ControlloVittoria(matrix, row, col) == 1){
            printf("<Server> Il Giocatore %d ha vinto!\n", request->turno +1);
            semOp(semid, MUTEX, -1);
            request->vittoria = request->turno;
            semOp(semid, MUTEX, 1);
        }else if(MatricePiena(matrix, row, col) == 1){   // la matrice è piena, la partita termina in parità
            printf("<Server> La partita si chiude in parità\n");
            semOp(semid, MUTEX, -1);
            request->vittoria = 2;  // il 2 rappresenta il pareggio
            semOp(semid, MUTEX, 1);
        }
        semOp(semid, VITTORIA, 1);

        semOp(semid, MUTEX, -1);
    }
    semOp(semid, MUTEX, 1); // mutex vittoria

    // il server prima di terminare aspetta i due client
    // altrimenti vengono rimossi i semafori mentre i client
    // sono ancora attivi

    semOp(semid, TERMINE, 0);

    free_shared_memory(request);
    free_shared_memory(matrix);
    remove_ipc();
    
    return 0;
}