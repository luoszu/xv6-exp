// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
acquire(struct spinlock *lk)
{
  pushcli(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    panic("acquire");

  // The xchg is atomic.
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  lk->cpu = cpu;
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both to not re-order.
  __sync_synchronize();

  // Release the lock.
  lk->locked = 0;

  popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
void
getcallerpcs(void *v, uint pcs[])
{
  uint *ebp;
  int i;

  ebp = (uint*)v - 2;
  for(i = 0; i < 10; i++){
    if(ebp == 0 || ebp < (uint*)KERNBASE || ebp == (uint*)0xffffffff)
      break;
    pcs[i] = ebp[1];     // saved %eip
    ebp = (uint*)ebp[0]; // saved %ebp
  }
  for(; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int
holding(struct spinlock *lock)
{
  return lock->locked && lock->cpu == cpu;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void
pushcli(void)
{
  int eflags;

  eflags = readeflags();
  cli();
  if(cpu->ncli == 0)
    cpu->intena = eflags & FL_IF;
  cpu->ncli += 1;
}

void
popcli(void)
{
  if(readeflags()&FL_IF)
    panic("popcli - interruptible");
  if(--cpu->ncli < 0)
    panic("popcli");
  if(cpu->ncli == 0 && cpu->intena)
    sti();
}

int sem_used_count=0;
struct sem sems[SEM_MAX_NUM];


void seminit () {             
    int i;
    for(i=0;i<SEM_MAX_NUM;i++){
         initlock(&(sems[i].lock), "semaphore");
         sems[i].allocated=0;
     }
     return; 
}



int sys_sem_create(void) {
    int n_sem, i;
    if(argint(0, &n_sem) < 0 )
        return -1;
    for(i = 0; i < SEM_MAX_NUM; i++) {
        acquire(&sems[i].lock);
        if(sems[i].allocated == 0) {
            sems[i].allocated = 1;
            sems[i].resource_count = n_sem;
            cprintf("create %d sem\n",i);
            release(&sems[i].lock);
            return i;
        }
        release(&sems[i].lock);
    }
    return -1;
 }


int sys_sem_free(){
    int id;    
    if(argint(0,&id)<0)        
      return -1;
    acquire(&sems[id].lock);    
    if(sems[id].allocated == 1 && sems[id].resource_count > 0){        
        sems[id].allocated = 0;        
        cprintf("free %d sem\n", id);    
    }    
    release(&sems[id].lock);
    return 0;
}

int sys_sem_p()
{       int id;
    if(argint(0, &id) < 0)
      return -1;
    acquire(&sems[id].lock);
    sems[id]. resource_count--;
    if(sems[id].resource_count<0)           //首次进入、或被唤醒时，资源不足
      sleep(&sems[id],&sems[id].lock);        //睡眠（会释放sems[id].lock才阻塞）
    release(&sems[id].lock);                                //解锁（唤醒到此处时，重新持有sems[id].lock）
    return 0;                                               //此时获得信号量资源
}

int sys_sem_v()
{       int id;
    if(argint(0,&id)<0)
      return -1;
    acquire(&sems[id].lock);
    sems[id]. resource_count+=1;            //增1
    if(sems[id].resource_count<1)                   //有阻塞等待该资源的进程
      wakeup1p(&sems[id]);                    //唤醒等待该资源的1个进程
    release(&sems[id].lock);                                //释放锁
    return 0;
}
