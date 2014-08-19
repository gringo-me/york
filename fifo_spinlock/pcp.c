#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h> /* for waitpid() */
#include <sys/mman.h>
#include <sched.h>

#include "tests.h"
#include "litmus.h"
static int *volatile mutex;

//TESTCASE(lock_pcp, P_FP,
//	 "PCP acquisition and release")
//{
//	int fd, od, cpu = 0;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	SYSCALL( sporadic_partitioned(ms2ns(10), ms2ns(100), cpu) );
//	SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//	SYSCALL( od = open_pcp_sem(fd, 0, cpu) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	/* tasks may not unlock resources they don't own */
//	SYSCALL_FAILS(EINVAL, litmus_unlock(od) );
//
//	SYSCALL( od_close(od) );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(pcp_inheritance, P_FP,
//	 "PCP priority inheritance")
//{
//	int fd, od, cpu = 0;
//
//	int child_hi, child_lo, child_middle, status, waiters;
//	lt_t delay = ms2ns(100);
//	double start, stop;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//	params.phase      = 0;
//	params.cls        = RT_CLASS_HARD;
//	params.budget_policy = NO_ENFORCEMENT;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//
//	child_lo = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		params.phase    = 0;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_pcp_sem(fd, 0, cpu) );
//
//		SYSCALL( wait_for_ts_release() );
//
//		SYSCALL( litmus_lock(od) );
//		start = cputime();
//		while (cputime() - start < 0.25)
//			;
//		SYSCALL( litmus_unlock(od) );
//
//		SYSCALL(sleep_next_period() );
//		);
//
//	child_middle = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY + 1;
//		params.phase    = ms2ns(100);
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//
//		SYSCALL( wait_for_ts_release() );
//
//		start = cputime();
//		while (cputime() - start < 5)
//			;
//		SYSCALL( sleep_next_period() );
//		);
//
//	child_hi = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY;
//		params.phase    = ms2ns(50);
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_pcp_sem(fd, 0, cpu) );
//
//		SYSCALL( wait_for_ts_release() );
//
//		start = wctime();
//		/* block on semaphore */
//		SYSCALL( litmus_lock(od) );
//		SYSCALL( litmus_unlock(od) );
//		stop  = wctime();
//
//		/* Assert we had some blocking. */
//		ASSERT( stop - start > 0.1);
//
//		/* Assert we woke up 'soonish' after the sleep. */
//		ASSERT( stop - start < 1 );
//
//		SYSCALL( kill(child_middle, SIGUSR2) );
//		SYSCALL( kill(child_lo, SIGUSR2) );
//		);
//
//	do {
//		waiters = get_nr_ts_release_waiters();
//		ASSERT( waiters >= 0 );
//	} while (waiters != 3);
//
//	SYSCALL( be_migrate_to_cpu(1) );
//
//	waiters = release_ts(&delay);
//
//	SYSCALL( waitpid(child_hi, &status, 0) );
//	ASSERT( status == 0 );
//
//	SYSCALL( waitpid(child_lo, &status, 0) );
//	ASSERT( status ==  SIGUSR2);
//
//	SYSCALL( waitpid(child_middle, &status, 0) );
//	ASSERT( status ==  SIGUSR2);
//}
//
//TESTCASE(srp_ceiling_blocking, P_FP | PSN_EDF,
//	 "SRP ceiling blocking")
//{
//	int fd, od;
//
//	int child_hi, child_lo, child_middle, status, waiters;
//	lt_t delay = ms2ns(100);
//	double start, stop;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//
//	SYSCALL( fd = open(".srp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//
//	child_lo = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		params.phase    = 0;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_srp_sem(fd, 0) );
//
//		SYSCALL( wait_for_ts_release() );
//
//		SYSCALL( litmus_lock(od) );
//		start = cputime();
//		while (cputime() - start < 0.25)
//			;
//		SYSCALL( litmus_unlock(od) );
//		);
//
//	child_middle = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY + 1;
//		params.phase    = ms2ns(100);
//		params.relative_deadline -= ms2ns(110);
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//
//		SYSCALL( wait_for_ts_release() );
//
//		start = cputime();
//		while (cputime() - start < 5)
//			;
//		);
//
//	child_hi = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY;
//		params.phase    = ms2ns(50);
//		params.relative_deadline -= ms2ns(200);
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_srp_sem(fd, 0) );
//
//		SYSCALL( wait_for_ts_release() );
//
//		start = wctime();
//		/* block on semaphore */
//		SYSCALL( litmus_lock(od) );
//		SYSCALL( litmus_unlock(od) );
//		stop  = wctime();
//
//		/* Assert we had "no" blocking (modulo qemu overheads). */
//		ASSERT( stop - start < 0.01);
//
//		SYSCALL( kill(child_middle, SIGUSR2) );
//		SYSCALL( kill(child_lo, SIGUSR2) );
//		);
//
//	do {
//		waiters = get_nr_ts_release_waiters();
//		ASSERT( waiters >= 0 );
//	} while (waiters != 3);
//
//	SYSCALL( be_migrate_to_cpu(1) );
//
//	waiters = release_ts(&delay);
//
//	SYSCALL( waitpid(child_hi, &status, 0) );
//	ASSERT( status == 0 );
//
//	SYSCALL( waitpid(child_lo, &status, 0) );
//	ASSERT( status ==  SIGUSR2);
//
//	SYSCALL( waitpid(child_middle, &status, 0) );
//	ASSERT( status ==  SIGUSR2);
//}
//
//TESTCASE(lock_dpcp, P_FP,
//	 "DPCP acquisition and release")
//{
//	int fd, od, cpu = 1;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	SYSCALL( sporadic_partitioned(ms2ns(10), ms2ns(100), 0) );
//	SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//	SYSCALL( od = open_dpcp_sem(fd, 0, cpu) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	/* tasks may not unlock resources they don't own */
//	SYSCALL_FAILS(EINVAL, litmus_unlock(od) );
//
//	SYSCALL( od_close(od) );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(lock_dpcp_pcp, P_FP,
//	 "DPCP-PCP interleaved priority")
//{
//	int fd, od_dpcp, od_pcp, child_hi, child_lo, status, waiters, cpu;
//	lt_t delay = ms2ns(100);
//	struct rt_task params;
//	double start;
//
//	/* tasks may not unlock resources they don't own */
//	SYSCALL( be_migrate_to_cpu(2) );
//
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(300);
//	params.period     = ms2ns(300);
//	params.relative_deadline = params.period;
//	params.phase      = 0;
//	params.cls        = RT_CLASS_HARD;
//	params.budget_policy = NO_ENFORCEMENT;
//
//	mutex = mmap(NULL, sizeof(*mutex), PROT_READ | PROT_WRITE,
//		                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
//	*mutex = 0;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	child_lo = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		params.phase = ms2ns(50);
//		params.cpu = partition_to_cpu(1);
//
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( init_rt_thread() );
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od_dpcp = open_dpcp_sem(fd, 0, partition_to_cpu(0)) );
//
//		SYSCALL( wait_for_ts_release() );
//
//		SYSCALL( litmus_lock(od_dpcp) );
//		cpu = sched_getcpu();
//		*mutex = 1;
//		SYSCALL( litmus_unlock(od_dpcp) );
//
//		/* Agent: Have I migrated? */
//		ASSERT( cpu == partition_to_cpu(0) );
//
//		SYSCALL(sleep_next_period() );
//		);
//
//	child_hi = FORK_TASK(
//		int preempted;
//		params.priority	= LITMUS_HIGHEST_PRIORITY;
//		params.phase = 0;
//		params.cpu = partition_to_cpu(0);
//
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( init_rt_thread() );
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od_pcp = open_pcp_sem(fd, 1, params.cpu) );
//
//		SYSCALL( wait_for_ts_release() );
//
//		/* block on semaphore */
//		SYSCALL( litmus_lock(od_pcp) );
//		start = cputime();
//		while (cputime() - start < 0.25)
//			;
//
//		preempted = *mutex;
//		SYSCALL( litmus_unlock(od_pcp) );
//
//		SYSCALL( od_close(od_pcp) );
//		ASSERT( preempted == 1 );
//
//		SYSCALL( kill(child_lo, SIGUSR2) );
//		);
//
//	do {
//		waiters = get_nr_ts_release_waiters();
//		ASSERT( waiters >= 0 );
//	} while (waiters != 2);
//
//	waiters = release_ts(&delay);
//
//	SYSCALL( waitpid(child_hi, &status, 0) );
//	ASSERT( status == 0 );
//
//	SYSCALL( waitpid(child_lo, &status, 0) );
//	ASSERT( status ==  SIGUSR2);
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//
//	munmap(mutex, sizeof(*mutex));
//}
//
//TESTCASE(not_lock_pcp_be, P_FP,
//	 "don't let best-effort tasks lock (D|M-)PCP semaphores")
//{
//	int fd, od;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	/* BE tasks are not even allowed to open a PCP semaphore */
//	SYSCALL_FAILS(EPERM, od = open_pcp_sem(fd, 0, 1) );
//
//	/* BE tasks are not allowed to open a D-PCP semaphore */
//	SYSCALL_FAILS(EPERM, od = open_dpcp_sem(fd, 0, 1) );
//
//	/* BE tasks are not allowed to open an M-PCP semaphore */
//	SYSCALL_FAILS(EPERM, od = open_mpcp_sem(fd, 0) );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//
//}
//
//TESTCASE(lock_mpcp, P_FP,
//	 "MPCP acquisition and release")
//{
//	int fd, od;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	SYSCALL( sporadic_partitioned(ms2ns(10), ms2ns(100), 0) );
//	SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//	SYSCALL( od = open_mpcp_sem(fd, 0) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	SYSCALL( litmus_lock(od) );
//	SYSCALL( litmus_unlock(od) );
//
//	/* tasks may not unlock resources they don't own */
//	SYSCALL_FAILS(EINVAL, litmus_unlock(od) );
//
//	SYSCALL( od_close(od) );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(pcp_lock_mode_change, P_FP,
//	 "PCP task becomes non-RT task while holding lock")
//{
//	int fd, od, cpu = 0;
//
//	int child, status;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//
//	child = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_pcp_sem(fd, 0, cpu) );
//
//		SYSCALL( litmus_lock(od) );
//
//		SYSCALL( task_mode(BACKGROUND_TASK) );
//
//		SYSCALL( litmus_unlock(od) );
//
//		SYSCALL( od_close(od) );
//
//		exit(0);
//		);
//
//	SYSCALL( waitpid(child, &status, 0) );
//	ASSERT( WIFEXITED(status) );
//	ASSERT( WEXITSTATUS(status) == 0 );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(mpcp_lock_mode_change, P_FP,
//	 "MPCP task becomes non-RT task while holding lock")
//{
//	int fd, od;
//
//	int child, status;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//
//	child = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mpcp_sem(fd, 0) );
//
//		SYSCALL( litmus_lock(od) );
//
//		SYSCALL( task_mode(BACKGROUND_TASK) );
//
//		SYSCALL( litmus_unlock(od) );
//
//		SYSCALL( od_close(od) );
//
//		exit(0);
//		);
//
//	SYSCALL( waitpid(child, &status, 0) );
//	ASSERT( WIFEXITED(status) );
//	ASSERT( WEXITSTATUS(status) == 0 );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(dpcp_lock_mode_change, P_FP,
//	 "DPCP task becomes non-RT task while holding lock")
//{
//	int fd, od, cpu = 0;
//
//	int child, status;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 1;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//
//	child = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_dpcp_sem(fd, 0, cpu) );
//
//		SYSCALL( litmus_lock(od) );
//
//		SYSCALL( task_mode(BACKGROUND_TASK) );
//
//		SYSCALL( litmus_unlock(od) );
//
//		SYSCALL( od_close(od) );
//
//		exit(0);
//		);
//
//	SYSCALL( waitpid(child, &status, 0) );
//	ASSERT( WIFEXITED(status) );
//	ASSERT( WEXITSTATUS(status) == 0 );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(pcp_lock_teardown, P_FP,
//	 "PCP task exits while holding lock")
//{
//	int fd, od, cpu = 0;
//
//	int child, status;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	exit(0);
//
//	child = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_pcp_sem(fd, 0, cpu) );
//
//		SYSCALL( litmus_lock(od) );
//		exit(123);
//		);
//
//	SYSCALL( waitpid(child, &status, 0) );
//	ASSERT( WIFEXITED(status) );
//	ASSERT( WEXITSTATUS(status) == 123 );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(dpcp_lock_teardown, P_FP,
//	 "DPCP task exits while holding lock")
//{
//	int fd, od, cpu = 0;
//
//	int child, status;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 1;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	exit(0);
//
//	child = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_dpcp_sem(fd, 0, cpu) );
//
//		SYSCALL( litmus_lock(od) );
//		exit(123);
//		);
//
//	SYSCALL( waitpid(child, &status, 0) );
//	ASSERT( WIFEXITED(status) );
//	ASSERT( WEXITSTATUS(status) == 123 );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(mpcp_lock_teardown, P_FP,
//	 "MPCP task exits while holding lock")
//{
//	int fd, od;
//
//	int child, status;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	exit(0);
//
//	child = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mpcp_sem(fd, 0) );
//
//		SYSCALL( litmus_lock(od) );
//		exit(123);
//		);
//
//	SYSCALL( waitpid(child, &status, 0) );
//	ASSERT( WIFEXITED(status) );
//	ASSERT( WEXITSTATUS(status) == 123 );
//
//	SYSCALL( close(fd) );
//
//	SYSCALL( remove(".pcp_locks") );
//}
//
//TESTCASE(mrsp_test, P_FP,
//	 "MRSP test")
//{
//	int fd, od = 0;
//
//	int child_hi, child_lo, status ;
//	int prio_per_cpu[4];
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(10000);
//	params.period     = ms2ns(100000);
//	params.relative_deadline = params.period;
//	params.phase      = 0;
//	params.cls        = RT_CLASS_HARD;
//	params.budget_policy = NO_ENFORCEMENT;
//	SYSCALL( be_migrate_to_cpu(3) );
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	prio_per_cpu [0] = 15 ;
//	prio_per_cpu [1] = 16 ;
//	prio_per_cpu [2] = 17 ;
//	prio_per_cpu [3] = 18 ;
//
//	child_lo = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		params.phase    = 0;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nLP task about to lock task\n");
//		SYSCALL( litmus_lock(od) );
//		printf("LP task locked task\n");
//		usleep(100);
//		SYSCALL( litmus_unlock(od) );
//		printf("LP task unlocked task\n");
//
//		);
//
//	params.cpu = 1;
//	child_hi = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY;
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nHP task about to lock task\n");
//		SYSCALL( litmus_lock(od) );
//		printf("HP task locked task\n");
//		SYSCALL( litmus_unlock(od) );
//		printf("HP task unlocked task\n");
//
//		);
//
//
//	SYSCALL( waitpid(child_hi, &status, 0) );
//	ASSERT( status == 0 );
//
//	SYSCALL( waitpid(child_lo, &status, 0) );
//	ASSERT( status == 0 );
//}
//TESTCASE(mrsp_many, P_FP,
//	 "MRSP 3 tasks one resource")
//{
//	int fd, od = 0;
//
//	int child_hi, child_lo, child_mid, status ;
//	int prio_per_cpu[4];
//	int i,j,k;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(100000);
//	params.period     = ms2ns(1000000);
//	params.relative_deadline = params.period;
//	//params.phase      = 0;
//	//params.cls        = RT_CLASS_HARD;
//	//params.budget_policy = NO_ENFORCEMENT;
//	SYSCALL( be_migrate_to_cpu(3) );
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	prio_per_cpu [0] = 10 ;
//	prio_per_cpu [1] = 15 ;
//	prio_per_cpu [2] = 20 ;
//	prio_per_cpu [3] = 25 ;
//
//	child_lo = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		params.phase    = 0;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nLP task about to lock task\n");
//		i = 10;
//		while (i){
//		i--;
//		SYSCALL( litmus_lock(od) );
//		printf("LP task locked task %d\n",i);
//		usleep(100000);
//		SYSCALL( litmus_unlock(od) );
//		printf("LP task unlocked task\n");
//		}
//		);
//
//	params.cpu = 1;
//	child_hi = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY;
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nHP task about to lock task\n");
//		j = 10;
//		while (j--){
//		SYSCALL( litmus_lock(od) );
//		printf("HP task locked task %d\n",j);
//		usleep(30000);
//		SYSCALL( litmus_unlock(od) );
//		printf("HP task unlocked task\n");
//		}
//		);
//
//	params.cpu = 2;
//	child_mid = FORK_TASK(
//		params.priority	= LITMUS_LOWEST_PRIORITY;
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nMP task about to lock task\n");
//		k = 10;
//		while (k--){
//		SYSCALL( litmus_lock(od) );
//		printf("MP task locked task %d\n",k);
//		usleep(20000);
//		SYSCALL( litmus_unlock(od) );
//		printf("MP task unlocked task\n");
//		}
//		);
//
//	SYSCALL( waitpid(child_hi, &status, 0) );
//	ASSERT( status == 0 );
//
//	SYSCALL( waitpid(child_lo, &status, 0) );
//	ASSERT( status == 0 );
//}
//TESTCASE(mrsp_four_tasks, P_FP,
//	 "MRSP 4 tasks one resource")
//{
//	int fd, od = 0;
//
//	int child_hi, child_lo, child_mid, child_fourth, status ;
//	int prio_per_cpu[4];
//	int i,j,k,l;
//
//	struct rt_task params;
//	init_rt_task_param(&params);
//	params.cpu        = 0;
//	params.exec_cost  =  ms2ns(100000);
//	params.period     = ms2ns(1000000);
//	params.relative_deadline = params.period;
//	//params.phase      = 0;
//	//params.cls        = RT_CLASS_HARD;
//	//params.budget_policy = NO_ENFORCEMENT;
//	SYSCALL( be_migrate_to_cpu(3) );
//
//	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );
//
//	prio_per_cpu [0] = 10 ;
//	prio_per_cpu [1] = 15 ;
//	prio_per_cpu [2] = 20 ;
//	prio_per_cpu [3] = 25 ;
//
//	child_lo = FORK_TASK(
//		params.priority = LITMUS_LOWEST_PRIORITY;
//		params.phase    = 0;
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nLP task about to lock task\n");
//		i = 10;
//		while (i){
//		i--;
//		SYSCALL( litmus_lock(od) );
//		printf("LP task locked task %d\n",i);
//		usleep(100000);
//		SYSCALL( litmus_unlock(od) );
//		printf("LP task unlocked task\n");
//		}
//		);
//
//	params.cpu = 1;
//	child_hi = FORK_TASK(
//		params.priority	= LITMUS_HIGHEST_PRIORITY;
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nHP task about to lock task\n");
//		j = 10;
//		while (j--){
//		SYSCALL( litmus_lock(od) );
//		printf("HP task locked task %d\n",j);
//		usleep(30000);
//		SYSCALL( litmus_unlock(od) );
//		printf("HP task unlocked task\n");
//		}
//		);
//
//	params.cpu = 2;
//	child_mid = FORK_TASK(
//		params.priority	= LITMUS_LOWEST_PRIORITY;
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nMP task about to lock task\n");
//		k = 10;
//		while (k--){
//		SYSCALL( litmus_lock(od) );
//		printf("MP task locked task %d\n",k);
//		usleep(20000);
//		SYSCALL( litmus_unlock(od) );
//		printf("MP task unlocked task\n");
//		}
//		);
//
//	params.cpu = 3;
//	child_fourth = FORK_TASK(
//		params.priority	= LITMUS_LOWEST_PRIORITY ;
//
//		SYSCALL( set_rt_task_param(gettid(), &params) );
//		SYSCALL( be_migrate_to_cpu(params.cpu) );
//		SYSCALL( task_mode(LITMUS_RT_TASK) );
//
//		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );
//
//		printf("\nFourth task about to lock task\n");
//		l = 10;
//		while (l--){
//		SYSCALL( litmus_lock(od) );
//		printf("Fourth task locked task %d\n",l);
//		usleep(20000);
//		SYSCALL( litmus_unlock(od) );
//		printf("Fourth task unlocked task\n");
//		}
//		);
//
//	SYSCALL( waitpid(child_hi, &status, 0) );
//	ASSERT( status == 0 );
//
//	SYSCALL( waitpid(child_lo, &status, 0) );
//	ASSERT( status == 0 );
//	
//	SYSCALL( waitpid(child_mid, &status, 0) );
//	ASSERT( status == 0 );
//	
//	SYSCALL( waitpid(child_fourth, &status, 0) );
//	ASSERT( status == 0 );
//}

TESTCASE(mrsp_paper_simulation, P_FP,
	 "MRSP 2 low tasks access resource one high")
{
	int fd, od = 0;

	int child_hi, child_lo, child_mid, status ;
	int prio_per_cpu[4];
	int i,j,k;
	int ii,jj,kk;

	struct rt_task params;
	init_rt_task_param(&params);
	params.cpu        = 0;
	params.exec_cost  =  ms2ns(100000);
	params.period     = ms2ns(1000000);
	params.relative_deadline = params.period;
	//params.phase      = 0;
	//params.cls        = RT_CLASS_HARD;
	//params.budget_policy = NO_ENFORCEMENT;
	SYSCALL( be_migrate_to_cpu(3) );

	SYSCALL( fd = open(".pcp_locks", O_RDONLY | O_CREAT, S_IRUSR) );

	prio_per_cpu [0] = 10 ;
	prio_per_cpu [1] = 15 ;
	prio_per_cpu [2] = 20 ;
	prio_per_cpu [3] = 25 ;

	child_lo = FORK_TASK(
		params.priority = LITMUS_HIGHEST_PRIORITY;
		params.phase    = 0;
		SYSCALL( set_rt_task_param(gettid(), &params) );
		SYSCALL( be_migrate_to_cpu(params.cpu) );
		SYSCALL( task_mode(LITMUS_RT_TASK) );

		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );

		printf("\nLP task about to lock task\n");
		i = 1000;
		while (i){
		i--;
		SYSCALL( litmus_lock(od) );
		printf("LP task locked task %d\n",i);
		
		ii = 0;

		while(ii < 10){ ii++; }
		SYSCALL( litmus_unlock(od) );
		printf("LP task unlocked task\n");
		}
		);

	params.cpu = 0;
	child_hi = FORK_TASK(
		usleep(200);
		params.priority	= LITMUS_LOWEST_PRIORITY;
		SYSCALL( set_rt_task_param(gettid(), &params) );
		SYSCALL( be_migrate_to_cpu(params.cpu) );
		SYSCALL( task_mode(LITMUS_RT_TASK) );

		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );

		printf("\nHP task about to lock task\n");
		j = 10;
		while (j--){
		jj = 0;

		while(jj < 10000000){jj++;}
		printf("HP iterates %d\n",j);
		}
		);

	params.cpu = 3;
	child_mid = FORK_TASK(

		params.priority	= LITMUS_HIGHEST_PRIORITY;
		SYSCALL( set_rt_task_param(gettid(), &params) );
		SYSCALL( be_migrate_to_cpu(params.cpu) );
		SYSCALL( task_mode(LITMUS_RT_TASK) );

		SYSCALL( od = open_mrsp_sem(fd, 0, prio_per_cpu) );

		printf("\nMP task about to lock task\n");
		k = 10;
		while (k--){
		SYSCALL( litmus_lock(od) );
		printf("MP task locked task %d\n",k);
		kk = 0;
		while(kk < 10){kk++;}
		SYSCALL( litmus_unlock(od) );

		printf("MP task unlocked task\n");
		}
		);

	SYSCALL( waitpid(child_hi, &status, 0) );
	ASSERT( status == 0 );

	SYSCALL( waitpid(child_lo, &status, 0) );
	ASSERT( status == 0 );
	
	SYSCALL( waitpid(child_mid, &status, 0) );
	ASSERT( status == 0 );
}
