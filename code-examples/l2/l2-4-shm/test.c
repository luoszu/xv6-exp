#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
int main(void)
{
  char *shm;
  int pid = fork();
  if(pid == 0){
    sleep(1);
    shm = (char*)shmgetat(1,3);//key为1，大小为3页的共享内存
    printf(1,"child  process pid:%d shm is %s refcount of 1 is:%d\n", getpid(), shm, shmrefcount(1));
    strcpy(shm, "hello_world!");
    printf(1, "child  process pid:%d write %s into the shm\n", getpid(), shm);
  } else if (pid > 0) {
    shm = (char*)shmgetat(1,3);
    printf(1,"parent process pid:%d before wait() shm is %s refcount of 1 is:%d\n", getpid(), shm, shmrefcount(1));
    strcpy(shm, "share_memory!");
    printf(1,"parent process pid:%d write  %s into the shm\n", getpid(), shm);
    wait();
    printf(1,"parent process pid:%d after wait() shm is %s refcount of 1 is:%d\n", getpid(), shm, shmrefcount(1));
  }
  exit();
}
