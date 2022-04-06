#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <string.h>

#define NUM_THREADS 12
#define MAX_LOG_MESSAGE 128
#define COURSE 1
#define ASSIGNMENT 1

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


void logMessage(const char* logMessage) 
{
    char message[MAX_LOG_MESSAGE+1];

    strncpy(message, logMessage, MAX_LOG_MESSAGE+1);
    if (message[MAX_LOG_MESSAGE] != '\0') {
	    message[MAX_LOG_MESSAGE] = '\0';
    }

    
    syslog(LOG_CRIT, "[COURSE:%d][ASSIGNMENT:%d] %s\n", COURSE, ASSIGNMENT, message);
}

void *counterThread(void *threadp)
{
    int sum=0, i;
    logMessage("Hello World from Thread!");
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=1; i < (threadParams->threadIdx)+1; i++)
        sum=sum+i;
 
    printf("Thread idx=%d, sum[0...%d]=%d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum);
}


int main (int argc, char *argv[])
{
   int rc;
   int i;

   logMessage("Hello World from Main!");

   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

   }

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
}
