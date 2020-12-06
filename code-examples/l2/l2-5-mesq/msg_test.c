#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"


struct msg{
  int type;
  char *dataaddr;
}s1,s2,g;

void msg_test()
{
  int mqid = mqget(123);
  int msg_len = 48;
  s1.dataaddr = "total number:47 : hello, this is child process.";
  int pid = fork(0);
  if(pid == 0){
    // s1.type = 1;
    // s1.dataaddr = "sdjfksdjkljg";
    // msgsnd(mqid, &s1, 13);
    // s1.type = 2;
    // s1.dataaddr = "l'ocem;i[a';D,F';SDLGP[ASDFSDGGAS";
    // msgsnd(mqid, &s1, 34);
    // s1.type = 3;
    // s1.dataaddr = "45SF45SD4G1FSD31G23SDF153A3SD1F3SAD13GF1SDA23G1ASD31G";
    // msgsnd(mqid, &s1, 54);
    sleep(10);
    for(int i=0; i<70; i++)
    {
      s1.type = i;
      msgsnd(mqid, &s1, msg_len);
    }
    printf(1,"发完\n");
  } else if (pid >0)
  {
    // sleep(10);     // sleep保证子进程消息写入
    g.dataaddr = malloc(msg_len);
    for(int i=0; i<70; i++)
    {
      g.type = i;
      msgrcv(mqid, &g, msg_len);
      printf(1, "读取第%d个消息： %s\n", i, g.dataaddr);

    }
    // g.dataaddr = malloc(54);
    // g.type = 2;
    // msgrcv(mqid,&g, 34);
    // printf(1, "-------\n");
    // printf(1, "读取第%d个消息： %s\n", 2, g.dataaddr);
    // printf(1, "-------\n");
    // g.type = 1;
    // msgrcv(mqid,&g, 13);
    // printf(1, "读取第%d个消息： %s\n", 1, g.dataaddr);
    // g.type = 3;
    // msgrcv(mqid,&g, 54);
    // printf(1, "读取第%d个消息： %s\n", 3, g.dataaddr);

    wait();

  }
  exit();
}


int
main(int argc, char *argv[])
{
  printf(1, "消息队列测试\n");
  msg_test();
  exit();
}