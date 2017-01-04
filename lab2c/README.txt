George Archbold
604407413
5/10/16
Project 2c

README


	In lab 2C, I included the same libraries as I did in the previous 
lab. I included stdio.h, stdlib.h, and string.h to accountfor all the 
standard c library functions and string functions like 
strcmp(). I included pthread.h so I could create and join the threads.
I also included getopt.h so that I could properly accept the different command line options. Lastly I included time.h to use the timer functions and SortedList.h to use the SortedList functions that I wrote in the .c file. 

	This program works similarly to project 2B, however it deals with 
more synchronization of hash tables instead of just linked lists. The program creates a specified number of nodes based on the iteration count times the number of threads. This initial array of nodes is then divided up among threads an each thread inserts the nodes into a specified amount of sublists, each of which is instantiated with a node head. The thread inserts the nodes by using a hash function on the node key. After all nodes are added to their respective sublists, the program finds the length of all the sublists combined and then goes through and deletes all the nodes from their sublist by using the same hash function location mechanism. By using the mutex and the spin lock the program ensures that no synchronization errors will occur between the threads. 

	I would like to use a LATE PASS on this lab allowing me to turn it
in on May 10th, 2016.



---------
Questions
---------

QUESTION 2C.1A
Explain the change in performance of the synchronized methods as a function of the number of threads per list.

By looking at the graph of timer per operation vs ratio of threads to lists for the mutex, the spin lock and for the unprotected case, I observed that the time per operation goes down as the ratio goes down. The more lists that are included in the program, the less contention there will be between threads and locks.

QUESTION 2C.1B
Explain why threads per list is a more interesting number than threads (for this particular measurement).

Threads per list is a more interesting number than threads, because threads alone do not determine the amount of contention in our program. The value for number of threads, is only important if you know how many lists there are. If there are less threads than lists it is less likely that there will be alot of contention. If there a more threads than lists, there is a greater chance that they will be trying to alter the same sublist. The threads compete for lock possesion, but this competition is based on the number of lists because when a thread needs to make a list alteration it is specific to the certain sublist. 


QUESTION 2C.2A
Compare the time per operation when increasing the lists value. Explain your observations.

When increasing the number of lists, the time per operation decreases. By looking at the mutex lock ran with 5 threads and 1000 iterations:

list size: 		time per op:
2 				1611
4 				986
5 				865
10 				631
15 				461
20 				334

This decrease in time per operation is due to the fact that there is less contention between the threads when there are more lists. It it less likely that 5 threads will try to access the same sublist when there are 20 sublists than when there are 5 threads trying to access 2 sublists.



QUESTION 2C.2B
Compare the time per operation between mutex and spinlock. Explain your observations.

As you can see from the graph, the spinlock and the mutex are very similar in their values for time per operation, however, mutex is a little bit faster. This is due to the fact that the mutex will waste a little less time since it will never waste CPU time spinning. Regardless the difference in their time per operations is almost negligible.

QUESTIONS 2C.3A
Why must the mutex be held when pthread_cond_wait is called?

The mutex must be held when pthread_cond_wait is called because pthread_cond_wait will give up its lock and put a thread to sleep until it recieves a signal to wake up. If it is not held prior to the pthread_cond_wait call, then this violates the way that the condition variable is used because there will be no lock to give up when thread is put to sleep.

QUESTION 2C.3B
Why must the mutex be released when the waiting thread is blocked?

The waiting thread must release the mutex while it is blocked so that another thread can use the mutex while the first thread is waiting for a signal that will allow it to return from the blocked state and continuing running. If the mutex was never released there would be no progress.

QUESTION 2C.3C
Why must the mutex be reacquired when the calling thread resumes?

The mutex must be reacquired when the calling thread resumes because the calling thread is awoken and will proceed to execute its critcal code and then release the lock to another waiting or blocked thread. If the mutex was not reacquired the calling thread mayb be unable to execute its code or it may cause race conditions and access memory that it is not supposed to.

QUESTION 2C.3D
Why must mutex release be done inside of pthread_cond_wait?  Why can't the caller simply release the mutex before calling pthread_cond_wait?

Mutex release must be done inside of pthread_cond_wait because that is the point at which the running thread is put to sleep and blocked. If the user was trusted to release the mutex, this could lead to problems where the thread may be put to sleep and the mutex was never released. In this case, the thread would be hold the lock for ever. The mutex must be released by pthread_cond_wait to avoid threads being put to sleep without allowing the next thread to run.



QUESTION 2C.3E
Can pthread_cond_wait be implemented in user mode?  If so, how?  If it can only be implemented by a system call, explain why?

Pthread_cond_wait can be implemented in user mode because it is implemented with a conidition variable and a mutex, which are both predefined synchronization objects. While manipulating the mutex and the condition variable may transfer priviledge to the kernel, the user can still control them in user mode.

-----------------------
Gprof Profiling Report: 
-----------------------

UNPROTECTED:

iterations: 1000
  %   cumulative   self              self     total
 time   seconds   seconds    calls  Ts/call  Ts/call  name
  0.00      0.00     0.00     2000     0.00     0.00  hash
  0.00      0.00     0.00     1000     0.00     0.00  SortedList_delete
  0.00      0.00     0.00     1000     0.00     0.00  SortedList_insert
  0.00      0.00     0.00     1000     0.00     0.00  SortedList_lookup
  0.00      0.00     0.00     1001     0.00     0.00  SortedList_length


iterations: 5000

  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 66.79      0.04     0.04     5000     8.02     8.02  SortedList_lookup
 33.40      0.06     0.02     5000     4.01     4.01  SortedList_insert
  0.00      0.06     0.00    10000     0.00     0.00  hash
  0.00      0.06     0.00     5000     0.00     0.00  SortedList_delete
  0.00      0.06     0.00     5001     0.00     0.00  SortedList_length

iterations: 10000

  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 63.08      0.17     0.17    10000    17.03    17.03  SortedList_lookup
 37.11      0.27     0.10    10000    10.02    10.02  SortedList_insert
  0.00      0.27     0.00    20000     0.00     0.00  hash
  0.00      0.27     0.00    10000     0.00     0.00  SortedList_delete
  0.00      0.27     0.00    10001     0.00     0.00  SortedList_length

  iterations: 100000
    %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 75.22     44.35    44.35   100000   443.55   443.55  SortedList_lookup
 24.91     59.04    14.69   100000   146.88   146.88  SortedList_insert
  0.03     59.06     0.02   200000     0.10     0.10  hash
  0.03     59.08     0.02                             main
  0.00     59.08     0.00   100000     0.00     0.00  SortedList_delete
  0.00     59.08     0.00   100001     0.00     0.00  SortedList_length



MUTEX:
  threads=5 iterations=1000

  lists=2
   %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 60.11      0.03     0.03     4792     6.27     6.27  SortedList_lookup
 40.07      0.05     0.02     4921     4.07     4.07  SortedList_insert
  0.00      0.05     0.00     9693     0.00     0.00  hash
  0.00      0.05     0.00     4842     0.00     0.00  SortedList_delete
  0.00      0.05     0.00       11     0.00     0.00  SortedList_length

  list=5
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 66.78      0.02     0.02     4177     4.80     4.80  SortedList_lookup
 33.39      0.03     0.01     4615     2.17     2.17  SortedList_insert
  0.00      0.03     0.00     8665     0.00     0.00  hash
  0.00      0.03     0.00     4203     0.00     0.00  SortedList_delete
  0.00      0.03     0.00       25     0.00     0.00  SortedList_length


list=10
   %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 50.09      0.01     0.01     4188     2.39     2.39  SortedList_insert
 50.09      0.02     0.01     3630     2.76     2.76  SortedList_lookup
  0.00      0.02     0.00     7726     0.00     0.00  hash
  0.00      0.02     0.00     3759     0.00     0.00  SortedList_delete
  0.00      0.02     0.00       43     0.00     0.00  SortedList_length

list=15
   %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
100.18      0.01     0.01     3217     3.11     3.11  SortedList_lookup
  0.00      0.01     0.00     7011     0.00     0.00  hash
  0.00      0.01     0.00     3940     0.00     0.00  SortedList_insert
  0.00      0.01     0.00     3347     0.00     0.00  SortedList_delete
  0.00      0.01     0.00       59     0.00     0.00  SortedList_length


list=20
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
100.18      0.01     0.01     3604     2.78     2.78  SortedList_insert
  0.00      0.01     0.00     7346     0.00     0.00  hash
  0.00      0.01     0.00     3888     0.00     0.00  SortedList_delete
  0.00      0.01     0.00     3747     0.00     0.00  SortedList_lookup
  0.00      0.01     0.00       76     0.00     0.00  SortedList_length


SPINLOCK:
  threads=5 iterations=1000

list=2
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 57.60      0.23     0.23                             threadfunc
 27.55      0.34     0.11     4653    23.68    23.68  SortedList_lookup
 15.03      0.40     0.06     4364    13.77    13.77  SortedList_insert
  0.00      0.40     0.00     9764     0.00     0.00  hash
  0.00      0.40     0.00     4922     0.00     0.00  SortedList_delete
  0.00      0.40     0.00       11     0.00     0.00  SortedList_length

list=5
   %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 40.07      0.06     0.06                             threadfunc
 33.39      0.11     0.05     3932    12.74    12.74  SortedList_lookup
 26.71      0.15     0.04     3603    11.12    11.12  SortedList_insert
  0.00      0.15     0.00     9018     0.00     0.00  hash
  0.00      0.15     0.00     4581     0.00     0.00  SortedList_delete
  0.00      0.15     0.00       24     0.00     0.00  SortedList_length

list=10
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 50.10      0.03     0.03                             threadfunc
 33.40      0.05     0.02     3535     5.67     5.67  SortedList_lookup
 16.70      0.06     0.01     3365     2.98     2.98  SortedList_insert
  0.00      0.06     0.00     8323     0.00     0.00  hash
  0.00      0.06     0.00     4349     0.00     0.00  SortedList_delete
  0.00      0.06     0.00       42     0.00     0.00  SortedList_length

list=15
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 33.39      0.01     0.01     3278     3.06     3.06  SortedList_insert
 33.39      0.02     0.01     3266     3.07     3.07  SortedList_lookup
 33.39      0.03     0.01                             threadfunc
  0.00      0.03     0.00     7659     0.00     0.00  hash
  0.00      0.03     0.00     4010     0.00     0.00  SortedList_delete
  0.00      0.03     0.00       60     0.00     0.00  SortedList_length

list=20
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
 50.09      0.01     0.01     3238     3.09     3.09  SortedList_lookup
 50.09      0.02     0.01     3149     3.18     3.18  SortedList_insert
  0.00      0.02     0.00     7077     0.00     0.00  hash
  0.00      0.02     0.00     3727     0.00     0.00  SortedList_delete
  0.00      0.02     0.00       77     0.00     0.00  SortedList_length


  In all the gprof profiles above, you can see how much time each function used as a percentage of the total runtime, how many calls were made to that function, and how long each function call took.







  


















  


















