// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint pcs[10];      // The call stack (an array of program counters)
                     // that locked the lock.
};

#define SEM_MAX_NUM 128


int sh_var_for_sem_demo;


extern int sem_used_count;


struct sem{
	struct spinlock lock;
	int resource_count;
	int pro_queue_len;
	int* procs; // 被阻塞的process
	int allocated;
};

extern struct sem sems[SEM_MAX_NUM];
