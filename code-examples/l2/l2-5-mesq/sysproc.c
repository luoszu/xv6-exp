#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
int 
sys_myalloc(void)
{
	int n;   // 分配 n 个字节
	if(argint(0, &n) < 0)
		return 0;
	if(n <= 0)
		return 0;
	return mygrowproc(n);
}

int 
sys_myfree(void) {
	int addr;
	if(argint(0, &addr) < 0)
		return -1;
	return myreduceproc(addr);
}


//messagequeue.c
int
sys_mqget(void)
{
  int key;
  if(argint(0,&key)<0)
    return -1;
  return mqget(key);
}

int
sys_msgsnd(void)
{
  int mqid;
  char* msg;
  int sz;
  if(argint(0, &mqid) < 0 || argint(2, &sz) < 0 || argptr(1,&msg,sz)<0)
    return -1;
  return msgsnd(mqid,msg,sz);
}

int
sys_msgrcv(void)
{
  int mqid;
  char* msg;
  int sz;
  if(argint(0, &mqid) < 0 || argint(2, &sz) < 0 || argptr(1,&msg,sz)<0)
    return -1;
  return msgrcv(mqid,msg,sz); 
}