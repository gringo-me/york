#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>

#include <list>
#include "fifo_spinlock.h"

#define LOOPS UCHAR_MAX

using namespace std;

list<int> the_list;
fifo_spinlock_t fifo_lock;

pthread_spinlock_t spinlock;
pthread_mutex_t mutex;

pid_t gettid() { return syscall( __NR_gettid ); }

void *consumer(void *ptr)
{
    int i;

    printf("Consumer TID %lu\n", (unsigned long)gettid());

    while (1)
    {
	fifo_spin_lock(&fifo_lock);	

        if (the_list.empty())
        {
#ifdef USE_SPINLOCK
	fifo_spin_unlock(&fifo_lock);
#endif
            break;
        }

        i = the_list.front();
        the_list.pop_front();

	fifo_spin_unlock(&fifo_lock);       
    }

    return NULL;
}

int main()
{
    int i;
    pthread_t thr1, thr2;
    struct timeval tv1, tv2;

    fifo_init_lock(&fifo_lock); 

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
