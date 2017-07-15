#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

/* Default problem size. */
#ifndef TSTEPS
# define TSTEPS 500
#endif
#ifndef N
# define N 500
#endif

/* Default data type is double. */
#ifndef DATA_TYPE
# define DATA_TYPE double
#endif
#ifndef DATA_PRINTF_MODIFIER
# define DATA_PRINTF_MODIFIER "%0.2lf "
#endif


DATA_TYPE A[N][N];

inline
void init_array()
{
  int i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      A[i][j] = ((DATA_TYPE) i*j + 10) / N;
}

inline
void print_array()
{
  int i, j;
    {
      for (i = 0; i < N; i++)
	for (j = 0; j < N; j++) {
	  printf(DATA_PRINTF_MODIFIER, A[i][j]);
	  if ((i * N + j) % 80 == 20) fprintf(stderr, "\n");
	}
      printf("\n");
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


int main(int argc, char** argv)
{
  int t, i, j;
  int tsteps = TSTEPS;
  int n = N;
  double t_start, t_end;

  /* Initialize array. */
  init_array();

  t_start = rtclock();

#pragma scop
#pragma live-out A

  for (t = 0; t <= tsteps - 1; t++)
    for (i = 1; i<= n - 2; i++)
      for (j = 1; j <= n - 2; j++)
	A[i][j] = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1]
		   + A[i][j-1] + A[i][j] + A[i][j+1]
		   + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/9.0;

#pragma endscop
 t_end = rtclock();
 printf("%lf\n", t_end - t_start); 
 printf ("%f, %f\n", A[1][1], A[N-2][N-2]);

  return 0;
}
