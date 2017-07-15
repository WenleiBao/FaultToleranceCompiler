#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>


/* Default problem size. */
#ifndef N
# define N 500
#endif

double A[N][N];
int B[N];

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
  // FILE *fp = fopen ("input.dat", "r");
  int i, j;
  float f;

  for (i = 0; i < N; i++)
  {
    B[i] = i;
    for (j = 0; j < N; j++)
     { 
        A[i][j] = rand() % 1000 + (i-j);
       // fscanf (fp, "%f ", &f);
       // A[i][j] = f;
     }
  }
}


int main(int argc, char** argv)
{
  int i, j, k;
  int n = N;

  /* Initialize array. */
  init_array();

  // Start timer
  double start, end;  
  start = rtclock();

#pragma scop
#pragma live-out A

  for (k = 0; k < n; k++)
  {
    for (j = k + 1; j < n; j++)
      A[k][j] = A[k][j] / A[k][k];
    for(i = k + 1; i < n; i++)
      for (j = k + 1; j < n; j++)
        A[i][j] = A[i][j] - A[i][k] * A[k][j];
  }

#pragma endscop
  // Stop timer and print elapsed time
  end = rtclock();
  printf ("%f\n", (end-start)); 
  printf ("%f, %f\n", A[n-1][n-1], A[1][1]);

  return 0;
}
