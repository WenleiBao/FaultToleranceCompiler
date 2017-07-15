#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>


/* Default problem size. */
#ifndef NI
# define NI 500
#endif
#ifndef NJ
# define NJ 500
#endif
#ifndef NK
# define NK 500
#endif


/* Default data type is double (dgemm). */
#ifndef DATA_TYPE
# define DATA_TYPE double
#endif

/* Array declaration. Enable malloc if POLYBENCH_TEST_MALLOC. */
DATA_TYPE alpha;
DATA_TYPE beta;
DATA_TYPE C[NI][NJ];
DATA_TYPE A[NI][NK];
DATA_TYPE B[NK][NJ];


inline
void init_array()
{
  int i, j;

  alpha = 32412;
  beta = 2123;
  for (i = 0; i < NI; i++)
    for (j = 0; j < NK; j++)
      A[i][j] = ((DATA_TYPE) i*j)/NI;
  for (i = 0; i < NK; i++)
    for (j = 0; j < NJ; j++)
      B[i][j] = ((DATA_TYPE) i*j + 1)/NJ;
  for (i = 0; i < NI; i++)
    for (j = 0; j < NJ; j++)
      C[i][j] = ((DATA_TYPE) i*j + 2)/NJ;
}

/* Define the live-out variables. Code is not executed unless
   POLYBENCH_DUMP_ARRAYS is defined. */
inline
void print_array()
{
  int i, j;
      for (i = 0; i < NI; i++) {
	for (j = 0; j < NJ; j++) {
	  fprintf(stderr, "%lf ", C[i][j]);
	}
	fprintf(stderr, "\n");
      }
}


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

int main(int argc, char** argv)
{
  int i, j, k;
  int ni = NI;
  int nj = NJ;
  int nk = NK;

  /* Initialize array. */
  init_array();

  // Start timer
  double start, end;  
  start = rtclock();

#pragma scop

  /* C := alpha*A*B + beta*C */
  for (i = 0; i < ni; i++)
    for (j = 0; j < nj; j++)
    {
      C[i][j] *= beta;
      for (k = 0; k < nk; ++k)
        C[i][j] += alpha * A[i][k] * B[k][j];
    }

#pragma endscop

  // Stop timer and print elapsed time
  end = rtclock();
  printf ("%f\n", (end-start)); 
  printf ("%f, %f\n", C[ni-1][nj-1], C[1][1]);

  return 0;
}
