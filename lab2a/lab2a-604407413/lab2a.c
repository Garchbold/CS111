#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>

#define BILLION 1000000000L

int opt_yield = 0;
long long counter = 0;
int threadcount = 1;
int itercount = 1;
pthread_mutex_t lock;
char sync;
volatile int spinlock = 0;
volatile int caslock = 0;


void * add(long long *pointer, long long value) {
  long long sum;
s  long long old;
  
    //mutex lock
    if(sync == 'm'){
      pthread_mutex_lock(&lock);
      sum = *pointer + value;
      if (opt_yield)
	pthread_yield();
      *pointer = sum;
      pthread_mutex_unlock(&lock);
    }

    
    //spin lock
    else if(sync == 's'){

      while(__sync_lock_test_and_set (&spinlock, 1) == 1){
	;
      }
      sum = *pointer + value;
      if (opt_yield)
	pthread_yield();
      *pointer = sum;
      __sync_lock_release(&spinlock);
    }

    //atomic
    else if(sync == 'c'){

      do{
	old = *pointer;
	sum = old + value;
	 if (opt_yield)
	   pthread_yield();
	 }
      while(!__sync_bool_compare_and_swap(&counter, old, sum));
	 
    }

    else{
      sum = *pointer + value;
      if (opt_yield)
	pthread_yield();
      *pointer = sum;
    }
}

void * threadfunc(){

  int i, j;
  for(i = 0; i < itercount; i++){
    add(&counter, 1);
  }

  for(j = 0; j < itercount; j++){
    add(&counter, -1);
  }
}

int main(int argc, char* argv[]){

  char c;
  int i = 0;
  int threadcheck = 0;

  struct timespec start, end;

  pthread_mutex_init(&lock, NULL);

  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
  
  while(1){
        static struct option long_options[] =
	  {
	    {"threads", required_argument, 0, 't'},
	    {"iterations", required_argument, 0, 'i'},
            {"yield", no_argument, &opt_yield, 1},
	    {"sync", required_argument, 0, 's'}
	  };

	int option_index = 0;
	c = getopt_long (argc, argv, "t:i:s:", long_options, &option_index);

	if (c == -1)
	  break;

	switch (c){

	case 't':
	  threadcount = atoi(optarg);
	  break;
	case 'i':
       	  itercount = atoi(optarg);
	  break;
	case 's':
	  sync = *optarg;
   	}
  }

  pthread_t pthreadid[threadcount];  

  clock_gettime(CLOCK_MONOTONIC, &start);
  
  for(i = 0; i < threadcount; i++){
  threadcheck = pthread_create( &pthreadid[i], &attribute, threadfunc, NULL);
    if(threadcheck)
      perror("Error ");
     }

  for(i = 0; i < threadcount; i++){
    threadcheck = pthread_join( pthreadid[i], NULL);
    if(threadcheck)
      perror("Error ");
  }
  
  clock_gettime(CLOCK_MONOTONIC, &end);
  
  int total = threadcount * itercount * 2;
  long long time = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  long long perop = time / total;
  
  printf("%d threads x %d iterations x (add + subtract) = %d operations\n", threadcount, itercount, total);
  if(counter != 0)
    fprintf(stderr, "ERROR: ");
  printf("final count = %d\n", counter);
  printf("elapsed time: %lld nanoseconds\n", time);
  printf("per operation: %lld nanoseconds\n", perop);
  
}
