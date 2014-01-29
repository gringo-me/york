

typedef struct fifo_spinlock_t {

volatile unsigned int owner;
volatile unsigned int next;

} fifo_spinlock_t;



/*When unsigned int reaches MAX_UINT it wraps up and starts from zero again*/

void fifo_init_lock(fifo_spinlock_t *lock){

	lock->owner = 0;
	lock->next = 0;
}


void fifo_spin_lock (fifo_spinlock_t * lock)
{
	unsigned int ticket;

	ticket = __sync_fetch_and_add(&lock->next,1);

	while (!(__sync_bool_compare_and_swap(&lock->owner, ticket,ticket)));

}

void fifo_spin_unlock (fifo_spinlock_t * lock)
{

	__sync_fetch_and_add(&lock->owner,1);
}



