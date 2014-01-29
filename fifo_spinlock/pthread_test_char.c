#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <limits.h>

#include <list>
#include <sched.h>

#include "char_fifo_spinlock.h"

#define LOOPS 5

using namespace std;

list<int> the_list;
fifo_spinlock_chart fifo_lock;

pthread_spinlock_t spinlock;
pthread_mutex_t mutex;

pid_t gettid() { return syscall( __NR_gettid ); }

/*When unsigned char reaches MAX_Uchar it wraps up and starts from zero again*/

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
	
	
	ticket = __sync_fetch_and_add(&lock->next,1);
	printf("Return getsched %d\n",pthread_getschedparam (pthread_self(), &policy, &param));
	printf("Ticket: %d prio %d\n",ticket,param.sched_priority);
	
	old_prio = param.sched_priority;	
	param.sched_priority = lock->prio;	

	printf("Prio return %d\n",pthread_setschedprio(pthread_self(),param.sched_priority));
 	pthread_getschedparam (pthread_self(), &policy, &param);
        printf("Ticket in loop: %d prio %d\n",ticket,param.sched_priority);

	while (!(__sync_bool_compare_and_swap(&lock->owner, ticket,ticket))){};
	 
	lock->task_prio = old_prio;
	
}

void fifo_spin_unlock_char(fifo_spinlock_chart * lock)
{
	struct sched_param param;
	int policy;

	param.sched_priority = lock->task_prio;

	//set back normal set prio
	pthread_setschedprio(pthread_self(),param.sched_priority);
	pthread_getschedparam (pthread_self(), &policy, &param);
	printf("Unlocking Ticket: prio %d\n",param.sched_priority);	
	__sync_fetch_and_add(&lock->owner,1);
}

/******************************************/



void *consumer(void *ptr)
{
    int i;

    printf("Consumer TID %lu\n", (unsigned long)gettid());

    while (1)
    {
	fifo_spin_lock_char(&fifo_lock);	

        if (the_list.empty())
        {

	fifo_spin_unlock_char(&fifo_lock);
            break;
        }

        i = the_list.front();
        the_list.pop_front();

	fifo_spin_unlock_char(&fifo_lock);       
    }

    return NULL;
}

int main()
{
    int i;
    pthread_t thr1, thr2;
    struct timeval tv1, tv2;
    struct sched_param param;

    fifo_init_lock_char(&fifo_lock);
    fifo_set_spinlock_ceiling(&fifo_lock, sched_get_priority_max(SCHED_FIFO)-1); 
    param.sched_priority = 3;
    sched_setscheduler(0, SCHED_FIFO, &param);
    
    printf("sched_max %d\n",sched_get_priority_max(SCHED_FIFO));

    // Creating the list content...
    for (i = 0; i < LOOPS; i++)
        the_list.push_back(i);

    // Measuring time before starting the threads...
    gettimeofday(&tv1, NULL);

	

    pthread_create(&thr1, NULL, consumer, NULL);
    pthread_create(&thr2, NULL, consumer, NULL);

    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL);

    // Measuring time after threads finished...
    gettimeofday(&tv2, NULL);

    if (tv1.tv_usec > tv2.tv_usec)
    {
        tv2.tv_sec--;
        tv2.tv_usec += 1000000;
    }

    printf("Result - %ld.%ld\n", tv2.tv_sec - tv1.tv_sec,
        tv2.tv_usec - tv1.tv_usec);


    return 0;
}










