// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
  int pid, wpid;

  if(open("console1", O_RDWR) < 0){
    mknod("console1", 1, 1);
    mknod("console2", 2, 2);
    //open("console1", O_RDWR); //0
  }
  // dup(0);  // stdout 1
  // dup(0);  // stderr 2

  for(;;){
    close(0);
    close(1);
    close(2);
    if(getcurconsole()==1){
      open("console1", O_RDWR); //0
    }else
    {
      open("console2", O_RDWR); //0
    }

    dup(0);
    dup(0);
    printf(1, "init: starting sh\n");
    pid = fork(0);
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid){
      printf(1, "zombie : pid %d, wpid %d!\n",pid,wpid);
    }
  }
}
