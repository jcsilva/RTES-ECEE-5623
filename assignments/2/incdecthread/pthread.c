#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <string.h>

#include <sys/utsname.h>

#define COUNT  1000

#define COURSE 1
#define ASSIGNMENT 2
#define MAX_LOG_MESSAGE 128

#define NUM_THREADS 128

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


// Unsafe global
int gsum = 0;


void logMessage(const char* logMessage) 
{
    char message[MAX_LOG_MESSAGE+1];

    strncpy(message, logMessage, MAX_LOG_MESSAGE+1);
    if (message[MAX_LOG_MESSAGE] != '\0') {
	    message[MAX_LOG_MESSAGE] = '\0';
    }

    
    syslog(LOG_CRIT, "[COURSE:%d][ASSIGNMENT:%d] %s\n", COURSE, ASSIGNMENT, message);
}

void printSystemInfo(void){
    char buff[100];
    struct utsname uname_pointer;

    uname(&uname_pointer);
  
    snprintf(buff, 100, "%s %s %s %s %s", uname_pointer.sysname, uname_pointer.nodename,  uname_pointer.release,  uname_pointer.version,  uname_pointer.machine);
    logMessage(buff);
}

void *incThread(void *threadp)
{
    int i;
    char buffer[50];
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
	snprintf(buffer, 50, "Increment thread idx=%d, gsum=%d", threadParams->threadIdx, gsum);
	logMessage(buffer);
    }
}

void *decThread(void *threadp)
{
    int i;
    char buffer[50];
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
	snprintf(buffer, 50, "Decrement thread idx=%d, gsum=%d", threadParams->threadIdx, gsum);
	logMessage(buffer);
    }
}



int main (int argc, char *argv[])
{
   int i=0;
   
   printSystemInfo();
   
   for (i=0; i<NUM_THREADS; ++i){
   	threadParams[i].threadIdx=i;
	pthread_create(&threads[i],   // pointer to thread descriptor
                  (void *)0,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   

   //	threadParams[i].threadIdx=i;
//	pthread_create(&threads[i], 
//			(void *)0, 
//			decThread, 
//			(void *)&(threadParams[i])
//			);
   }

   for(i=0; i<NUM_THREADS; i++)
     pthread_join(threads[i], NULL);

   printf("TEST COMPLETE\n");
}
