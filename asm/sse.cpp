#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <emmintrin.h>

using namespace std;

typedef unsigned long long LL;

inline LL rdtsc() __attribute__ ((always_inline));

const int w = 32, h = 32;
const int t = 10000;
const double c = 0.02;

const int rw = (w + 2) + (w + 2) % 2;
const int rh = (h + 2) + (h + 2) % 2;

int main()
{
    //freopen("output.txt", "w", stdout);

    double *a1 = new double[rw*rh+1];
    double *a2 = new double[rw*rh+1];

    double *p1 = (LL)a1 % 16 ? a1 : a1 + 1;
    double *p2 = (LL)a2 % 16 ? a2 : a2 + 1;

    for (int i = 0; i < rh*rh; i++)
        p1[i] = p2[i] = 1.0;

    p1[rw*3 + 3] = 1000000;
    double c2 = -4.0*c + 1;

    __m128d xc1 = _mm_set1_pd(c), xc2 = _mm_set1_pd(c2);

    unsigned long long start = rdtsc();
    for (int k = 0; k < t; k++) {
        double
            *pa = p1 + 1,
            *pb = pa + rw,
            *pc = pb + rw,
            *pr = p2 + rw + 1;
        for (int i = 1; i <= h; i++) {
            for (int j = 1; j <= w; j += 2) {
                asm (

                    "movapd (%1), %%xmm0\n\t"

                    "movapd (%0), %%xmm1\n\t"
                    "movapd (%2), %%xmm2\n\t"
                    "movupd -8(%1), %%xmm3\n\t"
                    "movupd 8(%1), %%xmm4\n\t"

                    "addpd %%xmm2, %%xmm1\n\t"
                    "addpd %%xmm4, %%xmm3\n\t"
                    "addpd %%xmm3, %%xmm1\n\t"

                    "mulpd %4, %%xmm1\n\t"
                    "mulpd %5, %%xmm0\n\t"

                    "addpd %%xmm1, %%xmm0\n\t"
                    
                    "movapd %%xmm0, (%3)\n\t"
                    :
                    : "r"(pa), "r"(pb), "r"(pc), "r"(pr),
                        "x"(xc1), "x"(xc2)
                    : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
                );
                pa += 2;
                pb += 2;
                pc += 2;
                pr += 2;
            }
            pa += 2;
            pb += 2;
            pc += 2;
            pr += 2;
        }
        swap(p1, p2);

        for (int i = 1; i <= h; i++) {
            for (int j = 1; j <= w; j++) {
                printf("%.1lf ", p1[rw*i + j]);
            }
            printf("\n");
        }
        getchar();
    }

    fprintf(stderr, "%llu\n", rdtsc() - start);

    delete[] a1;
    delete[] a2;

    return 0;
}

LL rdtsc(){
     LL a, d;
     asm volatile("rdtsc" : "=a" (a), "=d" (d));
     return a | (d << 32);
}
