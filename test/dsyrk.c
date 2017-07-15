#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
#include <sys/time.h>

#define NMAX 500
double a[NMAX][NMAX], c[NMAX][NMAX];


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
  double t_start, t_end;

  long N=NMAX;
  int i,j, k;

  for (i = 0; i < NMAX; i++) {
    for (j = 0; j < NMAX; j++) {
      c[i][j] = 0.0;
      a[i][j] = i*j*3.2345 / NMAX;
    }
  }

  t_start = rtclock();

#pragma scop
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      for (k=j; k<N; k++) {
        c[j][k] += a[i][j] * a[i][k];
      }
    }
  }
#pragma endscop


  t_end = rtclock();
  printf("%f\n", t_end - t_start);
  printf ("%f, %f, %f\n", c[0][0], c[N/2][N/2], c[N-1][N-1]);

  return 0;

}
