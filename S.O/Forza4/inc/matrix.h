#ifndef _MATRIX_HH
#define _MATRIX_HH

void stampaMatrice(char * matrix, int row, int col);
int valid_input(char * matrix, int input, int col);
int InserisciGettone(int colonna, char *M, int row, int col, char gettone);
int ControlloVittoria(char *M, int row, int col);

#endif

