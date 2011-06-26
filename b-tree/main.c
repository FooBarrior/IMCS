#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "linear_sequence_assoc.h"

#define DODO 13
int a[DODO + 1];

int main(){
    LSQ_HandleT h = LSQ_CreateSequence();
    LSQ_IteratorT it = NULL;
    int i = 0;
    float seed = time(NULL);
    printf("seed is 1299984384.00000030\n");
    srand(1299984384.00000030);
    for(i = 0; i < DODO; i++){
        a[i] = rand() % 100;
        printf("%d\n", a[i]);
        LSQ_InsertElement(h, a[i], i);
        printf("size %d\n", LSQ_GetSize(h));

        //LSQ_DumpSequence(h);
    }
    for(
        it = LSQ_GetPastRearElement(h), LSQ_RewindOneElement(it);
        !LSQ_IsIteratorBeforeFirst(it);
        LSQ_ShiftPosition(it, -1)
    ){
        printf("%d => %d\n", LSQ_GetIteratorKey(it), *LSQ_DereferenceIterator(it));
    }

    LSQ_DeleteElement(h, 123);
    for(i--; i >= 0; i--){
        printf("%d\n", a[i]);
        LSQ_DeleteFrontElement(h);
        printf("size %d\n", LSQ_GetSize(h));

        //LSQ_DumpSequence(h);
    }

    return EXIT_SUCCESS;
}
