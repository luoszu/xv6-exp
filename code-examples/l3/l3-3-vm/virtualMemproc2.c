// Create a zombie process that
// must be reparented at exit.

#include "types.h"
#include "stat.h"
#include "user.h"

void
mem(void){

    
    
    int pid = fork();

    if(pid == 0){
        char *a [346];
    
        for(int i = 1; i <= 345;i ++){
            char *s = (char*)sbrk(4096);//纯分配，不访问是不会分配物理空间的
            a[i] = s;
        }
        for(int i = 1; i <= 345;i ++){
            a[i][1] = 100; //开始分配347个页帧
        }   
        //循环访问最后2个 即344 345
        while(1){
            printf(1,"%s------\n","proc2");
            printf(1,"%d : %d \n",344,a[344][1]);
            printf(1,"%d : %d \n",345,a[345][1]);
            sleep(100 * 3);
            printf(1,"----------\n");
        }
    }else if(pid >0){
        char *a [348];
    
        for(int i = 1; i <= 347;i ++){
            char *s = (char*)sbrk(4096);//纯分配，不访问是不会分配物理空间的
            a[i] = s;
        }
        for(int i = 1; i <= 347;i ++){
            a[i][1] = 100; //开始分配347个页帧
        }
        //循环访问最后4个 即344 345 346 347
        while(1){
            printf(1,"%s------\n","proc1");
            printf(1,"%d : %d \n",344,a[344][1]);
            printf(1,"%d : %d \n",345,a[345][1]);
            sleep(100 * 3);
            printf(1,"%d : %d \n",346,a[346][1]);
            printf(1,"%d : %d \n",347,a[347][1]);
            printf(1,"----------\n");
        }
    
    }else{

    }

    

    
    
    

    





}
int
main(void)
{
    
    mem();
    // exit();
    return 0;
}