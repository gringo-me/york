#ifndef FIFO_CHAR_H
#define FIFO_CHAR_H


#include <sched.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>


pid_t gettid() { return syscall( __NR_gettid ); }

typedef struct fifo {
	volatile pid_t taskid;
	volatile unsigned char owner;
	volatile unsigned char next;
	volatile char *prio;
	volatile unsigned char task_prio;
	cpu_set_t saved_cpumask;

} fifo_spinlock_chart;

void fifo_init_lock_char(fifo_spinlock_chart *lock){

	lock->owner = 0;
	lock->next = 0;
}

void fifo_spin_lock_char(fifo_spinlock_chart * lock)
{
	unsigned char ticket;
	int policy,cpu,pid,old_prio,r;
	struct sched_param param;
	cpu_set_t mask, holder_mask;

	cpu = sched_getcpu();
	pid = gettid();

	printf("%d Entering function lock cpu %d\n",gettid(), cpu);
	// Getting a ticket to ensure FIFO
	ticket = __sync_fetch_and_add(&lock->next,1);
	printf("%d Calling getsched cpu %d\n",gettid(), cpu);
	pthread_getschedparam (pthread_self(), &policy, &param);
	// Saving original priority
	old_prio = param.sched_priority;
	printf("%d Ticket: %d prio %d cpu %d\n",gettid(),ticket,param.sched_priority, cpu);

	//param.sched_priority = lock->prio;
	// Raising priority to lock level	
	if(0 != pthread_setschedprio(pthread_self(),lock->prio[cpu]))
		printf("Thread %d sched_schedprio, error %d %s \n",gettid(),r,strerror(errno));
	 // Setting one priority per processor
	//syscall(SET_SMP_PRIO,pid,lock->prio);	
	
	// sched_getparam (0,&param);
	 
	printf("--Thread %d cpu %d priority %d should be %d--\n",gettid(),cpu,param.sched_priority, lock->prio[cpu]);	
	 printf("Thread %d is going to spin for lock cpu %d\n",gettid(), cpu);
	//spin
	while (!(__sync_bool_compare_and_swap(&lock->owner, ticket,ticket))){

		printf("Thread %d is SPINNING for lock (owner : %u, ticket %u) on cpu %d\n",gettid(),lock->owner, ticket, cpu);
		pthread_yield();
	}
	//if(0 != pthread_setschedprio(pthread_self(),lock->prio[cpu]+20))
	//	printf("Thread %d sched_schedprio, error %d %s \n",gettid(),r,strerror(errno));
	printf("Thread %d just got the lock setting taskid to %d cpu %d\n",gettid(),gettid(),cpu);
	// Setting the owner of the lock
	lock->taskid = gettid();
	lock->task_prio = old_prio;
}

void fifo_set_spinlock_smpceiling(fifo_spinlock_chart *lock, char *smp_prio){

	lock->prio = smp_prio;
}
void fifo_spin_unlock_char(fifo_spinlock_chart * lock)
{
	struct sched_param param;
	int policy;

	cpu_set_t mask;
	int cpu = sched_getcpu(); 
	// Memory barrier
	printf("Thread %d arrived at memory barrier %d\n",gettid(), cpu);
	__sync_synchronize();
	// Reinitialising owner of lock
	lock->taskid = -1;
	// Releasing lock
	printf("Thread %d about to release the lock %d\n",gettid(),cpu);
	__sync_fetch_and_add(&lock->owner,1);

	printf("Thread %d just unlocked the lock on cpu %d\n\n", gettid(),cpu);
	pthread_getschedparam (pthread_self(), &policy, &param);
	param.sched_priority = lock->task_prio;
	
	// Set back normal set prio
	printf("Thread %d setting back original priority cpu %d\n",gettid(),cpu);
//	pthread_setschedprio(pthread_self(),param.sched_priority);
}


#endif


