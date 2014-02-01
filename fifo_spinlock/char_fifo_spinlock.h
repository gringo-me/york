#ifndef FIFO_CHAR_H
#define FIFO_CHAR_H


#include <sched.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

pid_t gettid() { return syscall( __NR_gettid ); }
typedef struct fifo_spinlock_chart {
volatile pid_t  taskid;
volatile unsigned char owner;
volatile unsigned char next;
volatile unsigned char prio;
volatile unsigned char task_prio;
cpu_set_t saved_cpumask;

} fifo_spinlock_chart;

void fifo_init_lock_char(fifo_spinlock_chart *lock){

	lock->owner = 0;
	lock->next = 0;
}

void fifo_set_spinlock_ceiling(fifo_spinlock_chart *lock, unsigned char prio){

	lock->prio = prio;
}

void fifo_spin_lock_char(fifo_spinlock_chart * lock)
{
	unsigned char ticket;
	int policy;
	int old_prio;	
	struct sched_param param;
	cpu_set_t mask, holder_mask;	
	
	ticket = __sync_fetch_and_add(&lock->next,1);
	pthread_getschedparam (pthread_self(), &policy, &param);
	//printf("Ticket: %d prio %d\n",ticket,param.sched_priority);
	
	old_prio = param.sched_priority;	
	param.sched_priority = lock->prio;	
	//pthread_setschedprio(pthread_self(),param.sched_priority);

	sched_getaffinity(0, sizeof(mask), &mask);

        //if there is a holder add our current cpu affinity
	//printf("Thread %d What's the value of lock-taskid ? %d\n",gettid(),lock->taskid);	
	if(lock->taskid > 0){
	printf("Thread %d IN LOOP What's the value of lock-taskid ? %d\n",gettid(),lock->taskid);	
	
	   
	   printf("Thread %d Current cpu0 %d \n",gettid(),CPU_ISSET(0,&mask));	
           printf("Thread %d Current cpu1 %d \n",gettid(),CPU_ISSET(1,&mask));
	   //CPU_ZERO(&mask);
	   sched_getaffinity(lock->taskid,sizeof(mask), &holder_mask);
	   CPU_SET(sched_getcpu(), &holder_mask);
           int ret = sched_setaffinity(lock->taskid, sizeof(mask), &holder_mask);
	   if(0 > ret ){
		printf("Thread %d sched_affinity error %d %s \n",gettid(),ret,strerror(errno));
	      }
	}

	//spin	
	while (!(__sync_bool_compare_and_swap(&lock->owner, ticket,ticket))){};

	lock->taskid = gettid();
	//save our affinity to set it back later
	sched_getaffinity(0, sizeof(lock->saved_cpumask), &lock->saved_cpumask);		 
	lock->task_prio = old_prio;
	printf("Thread %d Set taskid to %d\n",gettid(),gettid());
	
}

void fifo_spin_unlock_char(fifo_spinlock_chart * lock)
{
	struct sched_param param;
	int policy;

	cpu_set_t mask;	

	//sleep(1);
	lock->taskid = -1;
	sched_getaffinity(0, sizeof(mask), &mask);
	printf("Thread %d unlock current cpu0 %d \n",gettid(),CPU_ISSET(0,&mask));	
        printf("Thread %d unlock current cpu1 %d \n",gettid(),CPU_ISSET(1,&mask));
	printf("Thread %d currently executing spin_unlock on cpu %d \n\n",gettid(),sched_getcpu());	

	//set back original affinity
	sched_setaffinity(gettid(), sizeof(lock->saved_cpumask), &lock->saved_cpumask);
	param.sched_priority = lock->task_prio;
	
	//set back normal set prio
	pthread_setschedprio(pthread_self(),param.sched_priority);
	__sync_fetch_and_add(&lock->owner,1);
}


#endif

