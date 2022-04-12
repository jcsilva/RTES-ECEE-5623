#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <sys/utsname.h>

#define COUNT  1000

#define COURSE 1
#define ASSIGNMENT 3
#define MAX_LOG_MESSAGE 128

#define NUM_THREADS 128

#define MY_SCHEDULER SCHED_FIFO

// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int rt_max_prio, rt_min_prio;
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;
pthread_attr_t main_attr;
pid_t mainpid;


typedef struct
{
    int threadIdx;
} threadParams_t;




void print_scheduler(void)
{
   int schedType, scope;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }

   pthread_attr_getscope(&main_attr, &scope);

   if(scope == PTHREAD_SCOPE_SYSTEM)
     printf("PTHREAD SCOPE SYSTEM\n");
   else if (scope == PTHREAD_SCOPE_PROCESS)
     printf("PTHREAD SCOPE PROCESS\n");
   else
     printf("PTHREAD SCOPE UNKNOWN\n");

}


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
    char buff[MAX_LOG_MESSAGE];
    struct utsname uname_pointer;

    uname(&uname_pointer);
  
    snprintf(buff, 100, "%s %s %s %s %s", uname_pointer.sysname, uname_pointer.nodename,  uname_pointer.release,  uname_pointer.version,  uname_pointer.machine);
    logMessage(buff);
}

void *incThread(void *threadp)
{
    int i;
    char buffer[MAX_LOG_MESSAGE];
    int sum = 0;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    unsigned cpu;

    for(i=1; i <= threadParams->threadIdx; ++i)
    {
        sum = sum+i;
    }

    cpu = sched_getcpu();

    snprintf(buffer, MAX_LOG_MESSAGE, "Thread idx=%d, sum=[1..%d]=%d Running on core: %d" , threadParams->threadIdx, threadParams->threadIdx, sum, cpu);
    logMessage(buffer);
}


int main (int argc, char *argv[])
{
   int i=0;
   threadParams_t threadParams[NUM_THREADS];

   mainpid = getpid();

   rt_max_prio=99;
   rt_min_prio=1;

   print_scheduler();
   sched_getparam(mainpid, &main_param);
   main_param.sched_priority=rt_max_prio;

   if(MY_SCHEDULER != SCHED_OTHER)
   {
       if(sched_setscheduler(getpid(), MY_SCHEDULER, &main_param) < 0)
	       perror("******** WARNING: sched_setscheduler");
   }

   print_scheduler();


   printf("rt_max_prio=%d\n", rt_max_prio);
   printf("rt_min_prio=%d\n", rt_min_prio);

   printSystemInfo();
   

   for(i=0; i < NUM_THREADS; i++)
   {
       pthread_attr_init(&rt_sched_attr[i]);
       pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
       pthread_attr_setschedpolicy(&rt_sched_attr[i], MY_SCHEDULER);
       

       rt_param[i].sched_priority= ((rt_min_prio+i) % rt_max_prio) + 1;
       pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],               // pointer to thread descriptor
                      &rt_sched_attr[i],         // use SCHEDULER attributes
                      incThread,              // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

   }


   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

   printf("\nTEST COMPLETE\n");

}
