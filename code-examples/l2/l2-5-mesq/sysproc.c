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
  int back;
  if(argint(0, &back) < 0)
    return -1;
  return fork(back);
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
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
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
    if(myproc()->killed){
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
sys_cps (void)
{
  return cps();
}

int
sys_shmgetat (void)
{
  int key, num;
  if(argint(0, &key) < 0 || argint(1, &num) < 0)
    return -1;
  return (int)shmgetat(key,num);
}

int
sys_shmrefcount(void)
{
  int key;
  if(argint(0,&key)<0)
    return -1;
  return shmrefcount(key);
}

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

int
sys_login(void)
{
  char* username, *pwd;
  if(argstr(0, &username) < 0 || argstr(1,&pwd)<0)
    return -1;
  return login(username,pwd);
}

int
sys_logout(void)
{
  logout();
  return 0;
}

int
sys_getcuruid(void)
{
  return getcuruid();
}

int
sys_getloginstate(void)
{
  return getloginstate();
}

int
sys_getcurconsole(void)
{
  return getcurconsole();
}

void
sys_changshell(void)
{
   changshell();
}

uint
sys_getticks(void)
{
  return ticks;
}
