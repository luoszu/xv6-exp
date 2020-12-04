#include "types.h"
#include "stat.h"
#include "user.h"
	
int
main(int argc, char *argv[])
{	int pid;
 	int data[8];
   printf(1,"This is a demo for prio-schedule!\n");
   pid=getpid();
   chpri(pid,19); //如果系统默认优先级不是19，则需先设置

      pid=fork();
      if(pid!=0){
          chpri(pid,15);                           //set 1st child’s prio=15
          printf(1,"pid%d prio%d\n",pid,15);
          pid=fork();
          if(pid!=0){
              chpri(pid,15);                       //set 2nd child’s prio=15
              printf(1,"pid%d prio%d\n ",pid,15);
              pid=fork();
              if(pid!=0){
                  chpri(pid,5);                     //set 3rd child’s prio=5
                  printf(1,"pid%d prio%d\n ",pid,5);
                  pid=fork();
                  if(pid!=0){
                      chpri(pid,5);                  //set 4th child’s prio=5
                      printf(1,"pid%d prio%d\n ",pid,5);
                   }
               }
            }
        }
   sleep(20);				//该睡眠是为了保证子进程创建完成，不是必须的 
        pid=getpid();
        printf(1,"pid=%d started\n",pid);

	int i,j,k;
        for( i=0;i<2;i++)
        {       printf(1,"pid=%d runing\n",pid);
                for( j=0;j<1024*100;j++)
                   for( k=0;k<1024;k++)
                      data[k%8]=pid*k;
         }
         printf(1,"pid=%d finished %d\n",pid,data[pid]);
        exit();
}

