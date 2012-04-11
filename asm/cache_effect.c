#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned long long LL;

inline LL rdtsc() __attribute__ ((always_inline)); //только хардкор
LL rdtsc(){
     LL a, d;
     asm volatile("rdtsc" : "=a" (a), "=d" (d));
     return a | (d << 32);
}

int main(){
    int sz = 128;
    for(int i = 0; i < 22; i++, sz <<= 1){
        char *c = malloc(sz);
        char *d = malloc(sz);
        LL s = rdtsc();
        memcpy(c, d, sz);
//        for(int j = 0; j < sz; j++){
//            d[j] = c[j];
//        }
        LL e = rdtsc() - s;
        printf("sz=%d,\ttime %lld;\t%.2f per byte\n", sz, e, ((double)e) / sz);
        free(d);
        free(c);
    }
}
