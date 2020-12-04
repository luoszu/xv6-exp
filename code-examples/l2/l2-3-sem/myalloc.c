#include "types.h"
#include "stat.h"
#include "user.h"


int
main(int argc, char *argv[]) {
  // int pid = getpid();   
  // map(pid);
  
  char* m1 = (char*)myalloc(2 * 4096);
  char* m2 = (char*)myalloc(3 * 4096);
  char* m3 = (char*)myalloc(1 * 4096);
  char* m4 = (char*)myalloc(7 * 4096);
  char* m5 = (char*)myalloc(9 * 4096);

  m1[1] = 'a';
  m2[2] = 'b';
  m3[2] = 'b';
  m4[2] = 'b';
  m5[2] = 'b';

  printf(1,"m1:%s\n",m1);
  myfree(m2);

  //m2[1] = 'p';

  myfree(m4);
  
  // map(pid);
  sleep(5000);
  myfree(m1);
  myfree(m3);
  myfree(m5);
  // char *p=(char *)0x0000;
  // for(int i=0x0000;i<0x08;i++)
  //     *(p+i)='*';

  // printf(1,"This string shouldn't be modified!\n");
  // exit();


  exit();
}
