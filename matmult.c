/*
 * group: Nico Stuhlmüller, Tobias Eckert
 * compile: gcc --std=c99 -lm -l pthread matmult.c
 * run: ./a.out <number threads> <size matrix>
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include<time.h>


typedef struct _tharg_t {
  double ** a;
  double ** b;
  double ** c;
  int N;
  int start_row;
  int stop_row;
  int start_col;
  int stop_col;
} tharg_t;

void * mult(void * arg){

  tharg_t *tharg  = (tharg_t *) arg;
  double ** a = tharg->a;
  double ** b = tharg->b;
  double ** c = tharg->c;
  int N = tharg->N;
  for(int i = tharg->start_row; i< tharg->stop_row; i++){
    for(int j= tharg->start_col; j < tharg->stop_col ; j++){
      c[i][j] = 0;
      for(int k = 0 ; k < N; k++){
        c[i][j] += a[i][k] * b[k][j];
      }
    }
  }
  return NULL;
}

void * mult_col(void * arg){
  tharg_t * tharg = (tharg_t *) arg;
  tharg->start_row=0;
  tharg->stop_row=tharg->N;
  mult(tharg);
  return NULL;
}

void * mult_row(void * arg){
  tharg_t * tharg = (tharg_t *) arg;
  tharg->start_col=0;
  tharg->stop_col=tharg->N;
  mult(tharg);
  return NULL;
}

double res(double ** A, int N, int i, int j){
  //analytische Lösung normalisiert
  return fabs(A[i][j] - 1./3*(i+2)*(j+1)*(N+1)*(N+2)*N)/A[i][j];
}

int min(int a, int b){
  if(a<b)
    return a;
  else
    return b;
}

void printMatrix(double ** a, int N){
  for (int i = 0; i < N; ++i) {
    for(int j = 0 ; j < N ; j++){
      printf("%f\t",a[i][j] );
    }
    printf("\n");
  }
}

int main(int argc, char *argv[]){
  clock_t start_time = clock();
  if(argc != 3){
    printf("wrong number of arguments!\n");
    return 1;
  }
  int p = atoi(argv[1]);
  const int N = atoi(argv[2]);
  if(p > N){
    printf("Numer of requested threads is higher than the number of rows of the matrix!\n");
    p = N;
    printf("Reducing threads to %d\n",p );
  }
  double ** a= ( double** ) malloc(N*sizeof(double));
  double ** b= ( double** ) malloc(N*sizeof(double));
  double ** c= ( double** ) malloc(N*sizeof(double));
  for(int i = 0;i<N;i++){
    a[i] = (double*) malloc(N*sizeof(double));
    b[i] = (double*) malloc(N*sizeof(double));
    c[i] = (double*) malloc(N*sizeof(double));
    for (int j= 0; j < N; ++j) {
      a[i][j] = (i+2) * (j+1);
      b[i][j] = (i+2) * (j+1);
    }
  }
  pthread_t tids[p];
  tharg_t args[p];
  int start = 0,stop = 0, exess = N%p;

  // Zeilenweise
  for(int i = 0 ; i < p ; i++){
    if (exess == 0) {
      stop += N/p;
    }else{
      stop += N/p + 1;
      exess -- ;
    }
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].N = N;
    args[i].start_row = start;
    args[i].stop_row = stop;
    args[i].start_col = 0;
    args[i].stop_col = N;
    pthread_create(& tids[i], NULL , mult_row , & args[i]);
    start=stop;
  }
  for(int i = 0 ; i < p ; i++){
    pthread_join(tids[i],NULL);
  }
  /* printMatrix(c,N); */

  //Spaltenweise
  start = 0; stop = 0; exess = N%p;
  for(int i = 0 ; i < p ; i++){
    if (exess == 0) {
      stop += N/p;
    }else{
      stop += N/p + 1;
      exess -- ;
    }
    args[i].a = a;
    args[i].b = b;
    args[i].c = c;
    args[i].N = N;
    args[i].start_col = start;
    args[i].stop_col = stop;
    args[i].start_row = 0;
    args[i].stop_row = N;
    pthread_create(& tids[i], NULL , mult_col , & args[i]);
    start=stop;
  }
  for(int i = 0 ; i < p ; i++){
    pthread_join(tids[i],NULL);
  }


  //testing
  /* printMatrix(c,N); */
  double sum = 0;
  for(int i = 0; i< N;i++){
    for(int j = 0 ; j<N ; j++){
      sum += res(c,N,i,j);
    }
  }
  printf("mean relative diviation per element %e\n", sum/(N*N));


  clock_t end_time = clock();
  float seconds = (float)(end_time - start_time) / CLOCKS_PER_SEC;
  printf("Time: %f sec\n", seconds);
  return 0;
}
