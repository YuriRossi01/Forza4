#ifndef _SHARED_MEMORY_HH
#define _SHARED_MEMORY_HH
#include <stdlib.h>
// struttura richiesta:
struct Request
{
    int shmid;
    int matrix[50][50];
    int row;
    int col;
    //char gettone1;
    //char gettone2;
};
int alloc_shared_memory(key_t key, size_t size);

void *get_shared_memory(int shmid, int shmflag);

void free_shared_memory( void *ptr_sh);

void remove_shared_memory(int shmid);

#endif
