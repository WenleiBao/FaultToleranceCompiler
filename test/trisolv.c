#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define NMAX 500

static double B[NMAX][NMAX], L[NMAX][NMAX];

void init_array()
{
    int i, j;

	for (i = 0; i < NMAX; i++) {
		for (j = 0; j < NMAX; j++) {
			B[i][j]  = i+j;
		    L[i][j] = (i+j+3.45) *i*j*0.5;
		}
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

void trisolv(long N) 
{
    long i,j,k;

#pragma scop
    for (i=0;i<=N-1;i++) {
        for (j=0;j<=N-1;j++) {
            for (k=0;k<=j-1;k++) {
                B[j][i]=B[j][i]-L[j][k]*B[k][i];  //S1 ;

            }
            B[j][i]=B[j][i]/L[j][j]; // S2 ;
        } // for j
    } // for i
#pragma endscop
}


int main()
{
    long N=NMAX;
    int i,j;
    double t_start, t_end;

    t_start = rtclock();
    trisolv(N);
    t_end = rtclock();
    printf("%f\n", t_end - t_start);

    printf ("%f, %f, %f\n", B[0][0], B[N/2][N/2], B[N-1][N-1]);
    return 0;
}
