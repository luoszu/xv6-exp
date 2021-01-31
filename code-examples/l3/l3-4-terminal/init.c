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
    mknod("console1", 1, 1);		//添加终端设备1
    mknod("console2", 2, 2);		//添加终端设备2
    //open("console1", O_RDWR); 	//这里暂时先不打开终端1设备文件
  }
  //dup(0);  // stdout 1			//暂时先不映射到1号文件描述符
  //dup(0);  // stderr 2			//暂时先不映射到2号文件描述符

  for(;;){
    close(0);						//当用户退出登录时会到达这里，需要释放原本的0，1，2号文件描述符
    close(1);
    close(2);
    if(getcurconsole()==1){			//如果为1号终端
      open("console1", O_RDWR);  	//打开1号终端设备，映射到文件描述符0
    }else
    {
      open("console2", O_RDWR); 	//打开2号终端设备，映射到文件描述符0
    }

    dup(0);						//同时映射到1号文件描述符
    dup(0);						//同时映射到2号文件描述符
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid) {
      printf(1, "zombie!\n");
	}
  }
}
