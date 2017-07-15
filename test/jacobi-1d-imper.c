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


double A[N];
double B[N];


static
inline
double rtclock()
{
    struct timezone Tzp;
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, &Tzp);
    if (stat != 0)
      printf("Error return from gettimeofday: %d", stat);
    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

inline
void init_array()
{
  int i, j;

  for (i = 0; i < N; i++)
    {
      A[i] = ((double) i + 10) / N;
      B[i] = ((double) i + 11) / N;
    }
}



int main(int argc, char** argv)
{
  int t, i, j;
  int tsteps = TSTEPS;
  int n = N;

  /* Initialize array. */
  init_array();

  // Start timer
  double start, end;  
  start = rtclock();

#pragma scop
#pragma live-out A

  for (t = 0; t < tsteps; t++)
    {
      for (i = 2; i < n - 1; i++)
	B[i] = 0.33333 * (A[i-1] + A[i] + A[i + 1]);

      for (j = 2; j < n - 1; j++)
	A[j] = B[j];
    }

#pragma endscop
  // Stop timer and print elapsed time
  end = rtclock();
  printf ("%f\n", (end-start)); 
  printf ("%f, %f\n", A[1], A[n-2]);

  return 0;
}
