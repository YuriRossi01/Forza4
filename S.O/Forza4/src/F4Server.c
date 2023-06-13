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
    int semid = semget(semkey, 9, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("semget failed");

    // Initialize the semaphore set
    union semun arg;
    // sem[0] sem[1] c in1 in2 mutex stampa
    unsigned short values[] = {1, 0, 0, 0, 1, 1, 0, 0, 2};
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

int ControlloVittoria(char *M, int row, int col){
    int i,j,x,y, count;
    
    for(i = row-1; i >= 0; i--)
        for(j = col-1; j >= 0; j--){
            if(*(M+(i*col)+j) != '/'){
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+((x-1)*col)+(y-1)) && x > 0 && y > 0){  //Controllo diagonale top sx - low dx
                    count++;
                    x--;
                    y--;
                    if(count == 4)
                        return 1;
                }
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+((x-1)*col)+(y+1)) && x > 0 && y < col-1){  //Controllo diagonale top dx - low sx          
                    count++;
                    x--;
                    y++;
                    if(count == 4)
                        return 1;
                }
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+(x*col)+(y-1)) && y > 0){   //Controllo la riga
                    count++;
                    y--;
                    if(count == 4)
                        return 1;
                }
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+((x-1)*col)+y) && x > 0){   //Controllo la colonna
                    count++;
                    x--;
                    if(count == 4)
                        return 1;
                }
            }
        }
        
    return 0;
}

int CambioTurno(int p1, int p2, int turno){
    if(turno == p1)
        turno = p2;
    else
        turno = p1;

    return turno;
}

void second_ctrlc(int sig){
    struct Request *request = get_shared_memory(ipc->shared_memory[1], 0);
    printf("\rGrazie per aver giocato!\n");
    remove_ipc();
    kill(request->pid[0], 2);
    kill(request->pid[1], 2);
    exit(0);
}

void first_ctrlc(int sig){
    printf("\r<Server> Premere nuovamente Ctrl-c per terminare l'esecuzione\n");
    signal(2, second_ctrlc);
}

void stampaMatrice(char * matrix, int row, int col){
    int i, j;
    printf("-------------------------------------------\n");
    for(i = 0; i < row; i ++){
        for(j = 0; j < col; j++)
            printf("%c ", *(matrix+(i*col)+j));
        printf("\n");
    }
    printf("-------------------------------------------\n");
}

// parametri:
// matrice, input, colonne max della matrice
int valid_input(char * matrix, int input, int col){
    return (input <= col) && (input >= 1) && (*(matrix + input - 1) == '/'); // matrix[0][input - 1]
}

void casual_game(){
    int semid = ipc->semaphore;
    int shmidServer = ipc->shared_memory[0];
    int shmidRequest = ipc->shared_memory[1];
    char * matrix = get_shared_memory(shmidServer, 0);
    struct Request *request = get_shared_memory(shmidRequest, 0);
    int index;
    int input;

    srand(time(NULL));


    // inizializzazione dei pid e degli indici
    semOp(semid, MUTEX, -1);
    index = request->i;
    request->pid[index] = getpid();
    request->i ++;
    semOp(semid, MUTEX, 1);

    semOp(semid, C, 1); // sblocco server

    stampaMatrice(matrix, request->row, request->col);

    // sblocco dell'altro giocatore che aspetta che player arrivi a 0
    semOp(semid, PLAYER, -1);

    while(!request->vittoria){
        if(request->turno != -1){ // stampa solo quando almeno uno ha fatto una giocata
            write(1, "Tocca all'avversario...\n", 24); // faccio con write a causa del buffer, 1 indica lo stdout
        }

        semOp(semid, index, -1); // P(sem[index])

        if(request->turno != -1){ // se turno = -1 vuol dire che il primo giocatore non ha ancora fatto la sua giocata
            printf("L'avversario ha giocato:\n");
            stampaMatrice(matrix, request->row, request->col);
        }

        semOp(semid, IN2, -1);

        // è il mio turno
        semOp(semid, MUTEX, -1);
        request->turno = index;
        semOp(semid, MUTEX, 1);

        semOp(semid, TURNO, 1);

        if(request->vittoria == 1)  //Quando sblocco il client in attesa del turno controllo se il giocatore prima di lui ha vinto
            printf("Hai perso\n");
        else {
            printf("<Server> E' il tuo turno\n");
            do{
                printf("<Server> Scegli in quale colonna giocare: ");
                input = rand() % request->col + 1;
                if(!valid_input(matrix, input, request->col))
                    printf("<Server> La colonna non è all'interno della matrice oppure hai inserito il gettone in una colonna piena!\n");
            }while(!valid_input(matrix, input, request->col));
            request->input = input;

            semOp(semid, IN1, 1);

            semOp(semid, STAMPA, -1);
            stampaMatrice(matrix, request->row, request->col);

            if(request->vittoria == 1)  //Se il client che ha eseguito la giocata ha vinto
                printf("Hai vinto!\n");
            else    //Altrimenti termino il turno
                printf("Turno terminato...\n");

            semOp(semid, (index == 1) ? 0 : 1, 1);
        }
        
    }
}

void casual_game_handler(int sig){
    printf("Inizio gioco casuale\n");

    int pid = fork();
    if(pid == 0){
        casual_game();
        exit(0);
    }
}

int main (int argc, char *argv[]) {

    ipc = (struct ipc_id *) malloc(sizeof(struct ipc_id));

    signal(2, first_ctrlc);
    signal(SIGUSR1, casual_game_handler);

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
        printf("<Server> argv[3] devi mettere i gettoni O o X\n");
        exit(1);
    }

    char gettone2 = *argv[4];
    if (gettone2 != G1 && gettone2 != G2 && gettone2 == gettone1){
        printf("<Server> argv[4] devi mettere i gettoni O o X\n");
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

    ipc->shared_memory[0] = shmidServer;
    ipc->shared_memory[1] = shmidRequest;
    
    printf("<Server> Collego la memoria condivisa per la matrice...\n");
    char *matrix = get_shared_memory(shmidServer, 0);
    struct Request *request = get_shared_memory(shmidRequest, 0);

    request->row = row;
    request->col = col;
    request->pid_server = getpid();
    request->vittoria = 0;
    
    printf("<Server> Matrice pronta... (%d %d)\n", request->col, request->row);

    // Inizializzo la matrice
    int i, j;
    for(i = 0; i < row; i++)
        for(j = 0; j < col; j++)
            *(matrix+(i*col)+j) = '/';

    // Inizializzazione semafori

    int vittoria = 0;
    key_t key_sem = ftok("./", 'c');
    int semid = create_sem_set(key_sem);
    int client;
    int turno;

    ipc->semaphore = semid;

    request->gettone = gettone1;
    printf("<Server> In attesa dei client\n");

    request->i = 0;
    request->turno = -1;

    semOp(semid, C, -1);
    printf("<Server> Player 1 collegato\n");

    semOp(semid, C, -1);
    printf("<Server> Player 2 collegato\n");

    // la variabile vittoria non serve se viene eseguito un exit nel ciclo

    while(!request->vittoria){
        // questo messaggio viene stampato due volte quando il primo player
        // fa la prima giocata, bisogna capire dove mettere questa istruzione

        semOp(semid, TURNO, -1);
        printf("<Server> In attesa di giocata di player %i\n", request->turno + 1);
        
        semOp(semid, IN1, -1); // P(IN1) blocco in attesa dell'input del client

        // controllo di chi è il turno per scegliere il gettone
        semOp(semid, MUTEX, -1); // mutex per accesso alla var condivisa turno
        if(request->turno == 0) {
            semOp(semid, MUTEX, 1);
            request->gettone = gettone1;
            client = 1;
        }
        else {
            semOp(semid, MUTEX, 1);
            request->gettone = gettone2;
            client = 2;
        }

        InserisciGettone(request->input, matrix, row, col, request->gettone);
        semOp(semid, STAMPA, 1);

        if(ControlloVittoria(matrix, row, col) == 1){   //Se il giocatore non ha vinto passo il turno
            //Segnlare ai client che il gioco è terminato indicandone i vincitori
            printf("<Server> Il Giocatore %d ha vinto!\n", client);
            request->vittoria = 1;

            // terminare gli altri due processi
            //exit(0);
        }
        semOp(semid, IN2, 1);

    }

    free_shared_memory(request);
    free_shared_memory(matrix);
    remove_ipc();
    
    return 0;
}