#include <stdio.h>
#include "../inc/matrix.h"

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

int InserisciGettone(int colonna, char *M, int row, int col, char gettone){ // giocatore, colonna inserimento, matrice 
    int i,j, ret = 0;
    
    for(i = row-1; i >= 0 && ret == 0; i--) // scorro la matrice dal basso
        for(j = col-1; j >= 0 && ret == 0; j--)
            if(j == colonna-1 && *(M+(i*col)+j) != 'X' && *(M+(i*col)+j) != 'O'){ // mi porto alla prima casella libera della colonna richiesta
                *(M+(i*col)+j) = gettone;
                ret = 1;    // gettone inserito
            }
    
    return ret; // ritorna 0 solo in caso di colonna piena
}

int ControlloVittoria(char *M, int row, int col){
    int i,j,x,y, count;
    
    for(i = row-1; i >= 0; i--)
        for(j = col-1; j >= 0; j--){
            if(*(M+(i*col)+j) != '/'){
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+((x-1)*col)+(y-1)) && x > 0 && y > 0){  // controllo diagonale top sx - low dx
                    count++;
                    x--;
                    y--;
                    if(count == 4)
                        return 1;
                }
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+((x-1)*col)+(y+1)) && x > 0 && y < col-1){  // controllo diagonale top dx - low sx          
                    count++;
                    x--;
                    y++;
                    if(count == 4)
                        return 1;
                }
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+(x*col)+(y-1)) && y > 0){   // controllo la riga
                    count++;
                    y--;
                    if(count == 4)
                        return 1;
                }
                count = 1;
                x = i;
                y = j;
                while(*(M+(x*col)+y) == *(M+((x-1)*col)+y) && x > 0){   // controllo la colonna
                    count++;
                    x--;
                    if(count == 4)
                        return 1;
                }
            }
        }
        
    return 0;
}

int MatricePiena(char *M, int row, int col){
    int i;

    for(i = 0; i < col; i++)    // scorro la prima riga della matrice, mi basta trovare un carattere non pieno per decretare la matrice non piena
        if(*(M+i) == '/') 
            return 0;

    return 1;
}
