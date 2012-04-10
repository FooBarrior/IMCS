#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef long long LL;

inline LL cpuid() __attribute__ ((always_inline)); //только хардкор
LL cpuid(){
     unsigned a, d;
     asm volatile("rdtsc" : "=a" (a), "=d" (d));

     return (((LL)a) | (((LL)d) << 32));
}

int main(){
    int sz = 128;
    for(int i = 0; i < 22; i++, sz <<= 1){
        char *c = malloc(sz * sizeof(char));
        char *d = malloc(sz);
        LL s = cpuid();
        memcpy(c, d, sz);
//        for(int i = 0; i < sz; i++){
//            d[i] = c[i];
//        }
        LL e = cpuid() - s;
        printf("sz=%d,\ttime %lld sec;\t%.2f on byte\n", sz, e, ((double)e) / sz);
        free(d);
//        free(c);
    }
}
