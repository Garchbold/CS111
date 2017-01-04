
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include "SortedList.h"

#define BILLION 1000000000L

int opt_yield;
int threadcount = 1;
int itercount = 1;
int listcount = 1;
pthread_mutex_t lock;
char sync;
volatile int spinlock = 0;
volatile int caslock = 0;
SortedListElement_t *init_array;

SortedList_t *hash_array;
int * hash_spinlock;
pthread_mutex_t * hash_mutex;

SortedList_t list;
SortedList_t *func_head = &list;


unsigned long hash(const char *str)
{
  unsigned long hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

void* threadfunc( void *arg){

  int length = 0;
  long k = 0;
  long start = (long) arg  * itercount;
  int hashaddress = 0;


    //INSERT
    for(k = start; k < start+itercount; k++){

      hashaddress = hash(init_array[k].key) % listcount;
      
      if(sync == 'm'){

	pthread_mutex_lock(&hash_mutex[hashaddress]);
	SortedList_insert(&hash_array[hashaddress], &init_array[k]);
	pthread_mutex_unlock(&hash_mutex[hashaddress]);
      }

      
      else if(sync == 's'){

	while(__sync_lock_test_and_set (&hash_spinlock[hashaddress], 1) == 1)
	  ;

	SortedList_insert(&hash_array[hashaddress], &init_array[k]);
	__sync_lock_release(&hash_spinlock[hashaddress]);
      }
      
      else	
	SortedList_insert(&hash_array[hashaddress], &init_array[k]);
    }



    int total = 0;
    int temp = 0;

      if(sync == 'm')
	{

	  for(int i = 0; i < listcount; i++)
	    {
	      pthread_mutex_lock(&hash_mutex[i]);

	      temp = SortedList_length(&hash_array[i]);
	      total = total + temp;
	      
	      pthread_mutex_unlock(&hash_mutex[i]);
	    }
	}

      else if(sync == 's')
	{

	  for(int i = 0; i < listcount; i++)
	    {

	      while(__sync_lock_test_and_set(&hash_spinlock[i], 1) == 1)
		;
	      temp = SortedList_length(&hash_array[i]);
	      total = total + temp;

	      __sync_lock_release(&hash_spinlock[i]);
	    }
	}
      else
	{
	  for(int i = 0; i < listcount; i++)
	    {
	      temp = SortedList_length(&hash_array[i]);
	      total = total + temp;

	    }

	}
    
    
    
    //DELETE
    for(k = start; k < start + itercount; k++){

      hashaddress = hash(init_array[k].key) % listcount;
      
      if(sync == 'm'){
        pthread_mutex_lock(&hash_mutex[hashaddress]);
	SortedList_delete(SortedList_lookup(&hash_array[hashaddress], init_array[k].key));
        pthread_mutex_unlock(&hash_mutex[hashaddress]);	
      }
      else if(sync == 's'){
	while(__sync_lock_test_and_set (&hash_spinlock[hashaddress], 1) == 1)
	  ;
	SortedList_delete(SortedList_lookup(&hash_array[hashaddress], init_array[k].key));
	__sync_lock_release(&hash_spinlock[hashaddress]);
      }
      else
	SortedList_delete(SortedList_lookup(&hash_array[hashaddress], init_array[k].key));
    }
   
}


int main(int argc, char* argv[]){

  char c;
  long long i = 0;
  long long j = 0;
  int threadcheck = 0;

  struct timespec start, end;
  pthread_mutex_init(&lock, NULL);

  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
  char* yield;
    
  while(1){
            static struct option long_options[] =
	      {
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", required_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{"lists", required_argument, 0, 'l'},
		{0, 0, 0, 0}
	      };

	    int option_index = 0;
	    c = getopt_long (argc, argv, "t:i:s:y:l:", long_options, &option_index);

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
	      break;
	      
	    case 'y':

	      yield = optarg;

	      while(*yield != '\0'){

		if(*yield == 'i')
		  opt_yield += INSERT_YIELD;

		if(*yield == 'd')
		  opt_yield += DELETE_YIELD;

		if(*yield == 's')
		  opt_yield += SEARCH_YIELD;  

		yield++;
	      }
	      break;
	      
	    case 'l':
	      listcount = atoi(optarg);
	      hash_array = malloc(listcount * sizeof(SortedList_t));
	      hash_spinlock = malloc(listcount*sizeof(int));
	      hash_mutex = malloc(listcount*sizeof(pthread_mutex_t));
	      
	      int d;
	      for(d = 0; d < listcount; d++)
		{
		  hash_array[d].next = &hash_array[d];
		  hash_array[d].prev = &hash_array[d];
		  hash_array[d].key = 0;
		  hash_spinlock[d] = 0;
		  pthread_mutex_init(&hash_mutex[d], NULL);
		}
	      break;
	    }
  }

  func_head->next = func_head;
  func_head->prev = func_head;
  func_head->key = 0;
  
  int n = threadcount * itercount;
  int keylen = 10;
  int keylistsize = n* (keylen + 1);
  char* keylist = (char*)malloc(keylistsize * sizeof(char));
  int nchars = 1 + 'z' - '0';
  int counter = 0;
 

  for(i = 0; i < keylistsize; i++){
      counter++;
      if(counter % (keylen + 1) == 0)
	keylist[i] ='\0';
      else
	keylist[i]= '0' + rand() % nchars; //RAND_MAX
  }

  init_array = malloc(n*sizeof(SortedListElement_t));
  
  for(i = 0; i < n; i++){
    SortedListElement_t new_element;
    new_element.key = keylist;
    keylist += keylen+1;
    init_array[i] = new_element;
    }

  pthread_t pthreadid[threadcount];
  clock_gettime(CLOCK_MONOTONIC, &start);

  for(i = 0; i < threadcount; i++){
    threadcheck = pthread_create( &pthreadid[i], &attribute, threadfunc, (void *)i);

    if(threadcheck)
      perror("Error ");
  }

  for(j = 0; j < threadcount; j++){
    threadcheck = pthread_join( pthreadid[j], NULL);
    if(threadcheck)
      perror("Error ");
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  int total = threadcount * itercount * 2;
  long long time = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  long long perop = time / total;

  printf("%d threads x %d iterations x (add + subtract) = %d operations\n", threadcount, itercount, total);
  if(SortedList_length(func_head) != 0)
    printf("final count = %d\n", SortedList_length(func_head));
  printf("elapsed time: %lld nanoseconds\n", time);
  printf("per operation: %lld nanoseconds\n\n", perop);

}
