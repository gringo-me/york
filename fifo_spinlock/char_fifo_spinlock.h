

typedef struct fifo_spinlock_chart {

volatile unsigned char owner;
volatile unsigned char next;
volatile unsigned char prio;
volatile unsigned char task_prio;
} fifo_spinlock_chart;







