#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#define TMAX 500
#define NMAX 500

#define T TMAX
#define N NMAX


static double X[NMAX][NMAX+13], A[NMAX][NMAX+23], B[NMAX][NMAX+37];

void init_array()
{
    int i, j;

    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            A[i][j] = (1+(i*j)%1024)/2.0;
            B[i][j] = (1+(i*j)%1024)/3.0;
            X[i][j] = (1+(i*j)%1024)/3.0;
        }
    }
}


void print_array()
{
    int i, j;

    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            fprintf(stdout, "%lf ", B[i][j]);
            if (j%80 == 79) fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
    }
}

double rtclock()
{
    struct timezone Tzp;
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, &Tzp);
    if (stat != 0) printf("Error return from gettimeofday: %d",stat);
    return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}

int main()
{
    int i, j, k, l, m, n, t;

    int i1, i2;

    double t_start, t_end;

    init_array();

#ifdef PERFCTR
    PERF_INIT; 
#endif

    t_start = rtclock();

#pragma scop
    for (t = 0; t < T; t++) {

        for (i1=0; i1<N; i1++) {
            for (i2 = 1; i2 < N; i2++) {
                X[i1][i2] = X[i1][i2] - X[i1][i2-1] * A[i1][i2] / B[i1][i2-1];
                B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] / B[i1][i2-1];
            }
        }

        for (i1=1; i1<N; i1++) {
            for (i2 = 0; i2 < N; i2++) {
                X[i1][i2] = X[i1][i2] - X[i1-1][i2] * A[i1][i2] / B[i1-1][i2];
                B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] / B[i1-1][i2];
            }
        }
    }
#pragma endscop

    t_end = rtclock();
    printf("%lf\n", t_end - t_start);

    //    print_array();
    printf ("%f, %f, %f, %f\n", B[N-2][N-2], B[1][1], X[N-2][N-2], X[1][1]);

    return 0;
}
