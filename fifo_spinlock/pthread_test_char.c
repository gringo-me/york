#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <stdlib.h>

#include <errno.h>
#include <stdarg.h>
#include "char_fifo_spinlock.h"

#define LOOPS 5


fifo_spinlock_chart fifo_lock;

pthread_spinlock_t spinlock;
pthread_mutex_t mutex;

//pid_t gettid() { return syscall( __NR_gettid ); }

/*When unsigned char reaches MAX_Uchar it wraps up and starts from zero again*/

static int mark_fd = -1;
static __thread char buff[BUFSIZ+1];
/******************************************/

static char prio_per_cpu[4] = {119,118,118,117};

static char buffer[BUFSIZ];
int cpu = 0;

static void perr(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, BUFSIZ, fmt, ap);
	va_end(ap);

	perror(buffer);
	fflush(stderr);
	exit(-1);
}

static void setup_ftrace_marker(void)
{
	struct stat st;
	char *files[] = {
		"/sys/kernel/debug/tracing/trace_marker",
		"/debug/tracing/trace_marker",
		"/debugfs/tracing/trace_marker",
	};
	int ret;
	int i;

	for (i = 0; i < (sizeof(files) / sizeof(char *)); i++) {
		ret = stat(files[i], &st);
		if (ret >= 0)
			goto found;
	}
	/* todo, check mounts system */
	return;
found:
	mark_fd = open(files[i], O_WRONLY);
}

static void ftrace_write(const char *fmt, ...)
{
	va_list ap;
	int n;

	if (mark_fd < 0)
		return;

	va_start(ap, fmt);
	n = vsnprintf(buff, BUFSIZ, fmt, ap);
	va_end(ap);

	write(mark_fd, buff, n);
}
void *consumer(void *ptr)
{
	int i;
	int j = 0;
	long pid;
	int cpu;

	printf("Consumer TID %lu\n", (unsigned long)gettid());

	cpu_set_t cpumask;
	CPU_ZERO(&cpumask);
	CPU_SET(cpu, &cpumask); cpu++;
	sched_setaffinity(0, sizeof(cpumask), &cpumask);

	pid = gettid();

	printf("Thread %d id started \n",pid);

	for (i = 0; i < LOOPS; i++){
		cpu = sched_getcpu();

		fifo_spin_lock_char(&fifo_lock);	
		//we force thread migration by changing cpu affinities
		cpu++;	  
		if (cpu > 3) cpu = 0;

		CPU_ZERO(&cpumask);
		CPU_SET(cpu, &cpumask); 
		printf("MSS531:pid %d next cpu %d \n",pid,cpu);	

		fifo_spin_unlock_char(&fifo_lock);       
	}
	printf("Thread %d currently executing in cpu %d \n", gettid(),sched_getcpu());
	return NULL;
}

int main()
{
    int i;
    pthread_t thr1, thr2;
    pthread_t *threads;
    struct timeval tv1, tv2;
    struct sched_param param;
    long *thread_pids;
    int nr_tasks = 1;

    fifo_init_lock_char(&fifo_lock);
    fifo_set_spinlock_smpceiling(&fifo_lock, prio_per_cpu); 
    param.sched_priority = 110;
    sched_setscheduler(0, SCHED_FIFO, &param);
    
    printf("sched_max %d\n",sched_get_priority_max(SCHED_FIFO));

    // Measuring time before starting the threads...
    gettimeofday(&tv1, NULL);
	
    threads = (pthread_t*) malloc(sizeof(*threads) * nr_tasks);
    thread_pids = (long*) malloc(sizeof(long) * nr_tasks);
    
    for (i=0; i < nr_tasks; i++) {
	    if (pthread_create(&threads[i], NULL, consumer, NULL))
		    perr("pthread_create");
    }



	for (i=0; i < nr_tasks; i++)
		pthread_join(threads[i], (void**)&thread_pids[i]);
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










