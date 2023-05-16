#include <stdio.h>
#define Row 5
#define Col 5
void initialieBoard(char board[Row][Col]);
void printBoard(char board[Row][Col]);
int main(int argc, char const *argv[])
{
    char board[Row][Col];
    initialieBoard(board);
    printBoard(board);
    

    return 0;
}

void initialieBoard(char board[Row][Col]){
    for (int i = 0; i < Row; i++)
    {
        for (int j = 0; j < Col; j++)
        {
            board[i][j] = ' ';
        }
        
    }
    
}

void printBoard(char board[Row][Col]){
    for (int i = 0; i < Row; i++)
    {
        for (int j = 0; j < Col; j++)
        {
            printf("|%c",board[i][j]);
        }
        printf("|\n");
    }
    printf("-----------\n");
    for (int i = 0; i < Col; i++)
    {
        printf(" %d",i);
    }
}