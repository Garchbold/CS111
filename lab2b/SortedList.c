#define _GNU_SOURCE

#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;

extern int opt_yield;

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *The specified element will be inserted in to
 *the specified list, which will be kept sorted
 *in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 *
 * Note: if (opt_yield & INSERT_YIELD)
 *call pthread_yield in middle of critical section
 */

//INSERT
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){

  SortedList_t *temp;
  temp = list;

  //if list is only head
  if(list->next == list){
    list->prev = element;
    list->next = element;
    element->prev = list;
    element->next = list;
    return;
  }

  while(temp->next != list){

    
    temp = temp->next;
    //if element goes in the middle of the list
    if(strcmp(element->key, temp->key) < 0){

      if(opt_yield == 1 || opt_yield == 3 || opt_yield == 5 || opt_yield == 7)
	pthread_yield();
   
      temp->prev->next=element;
      element->next=temp;
      element->prev=temp->prev;
      temp->prev=element;
      return;
    }

  }

  //if element goes at the end of the list
  if(strcmp(element->key,temp->key) > 0){
    temp->next = element;
    element->prev = temp;
    element->next = list;
    list->prev = element;
    return;
  }
}

//DELETE
int SortedList_delete( SortedListElement_t *element){

  if((element->prev->next != element) || (element->next->prev != element))
    return 1;
  //  printf("going to delete %s\n", element->key);
  if(element->key != 0){

    if(opt_yield == 2 || opt_yield == 3 || opt_yield == 6 || opt_yield == 7)
      pthread_yield();

    element->prev->next = element->next;
    element->next->prev = element->prev;
    //printf("deleted node \n");
    return 0;
  }
}

//LOOKUP
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){

  
  SortedList_t *temp;


  temp = list->next;
  //printf("LOOKUP\n");
  while( temp != list){


    if(temp->key == key){
      //      printf("The node found is %s \n", temp->key);
      //printf("The node to be deleted is %s \n", key);
    if(opt_yield == 3 || opt_yield == 4 || opt_yield == 5 || opt_yield == 7)
      pthread_yield();

      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}


//LENGTH
int SortedList_length(SortedList_t *list){
  SortedList_t *temp;
  int counter = 0;
  temp = list;

  if(temp->next == list)
    return 0;

  temp = temp->next;
  while(temp->next != list)
    {
      if(opt_yield == 3 || opt_yield == 4 || opt_yield == 5 || opt_yield == 7)
	pthread_yield();

      counter++;
      temp = temp->next;
    }
  counter++;
  return counter;
}
