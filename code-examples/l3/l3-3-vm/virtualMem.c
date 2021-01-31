/*
    虚拟内存实验
*/
/*
实验思路：
1.先把物理页数量减少至1，便于观察实验
2.剩余1个页，我们要在1个页的基础上再给进程分配10个页（即10*4096的进程空间)
3.循环访问分配的10个页：每次访问都先要换出一个页，再把之前写到磁盘的页换入

实验效果：
        1.物理页从剩余数量为690，减少至1
        2.除第一次分配不用换出外，后面的9次分配都要换出，一共换出9次
        3.每次访问，先换出一个页(腾出一个物理页),再换入之前在磁盘的物理页数据
*/

#include "types.h"
#include "stat.h"
#include "user.h"
void
mem(void){
    char *adTable[11]; // 存储页的第一个地址
    for(int i = 0; i < 4; i++){
            adTable[i] = sbrk(4096);// 延迟分配页帧
    }
    printf(1,"---------------------------------------\n");  
    for(int i = 0 ; i < 4; i ++){
        adTable[i][1] = 'a' + i ; //这时候就出现缺页，需要换出一个页(腾出一个位置) ，并且给页的第一个字节赋值为字母a~d
    }
    printf(1,"---------------------------------------\n");  
    printf(1,"访问第4个页，其内容是 %c, 不发生缺页异常\n\n",adTable[3][1]);
    printf(1,"下面展示缺页异常的交换功能:\n");
    for(int i = 0,cnt = 1; cnt <= 4; i= (i+1) % 4 ,cnt++){
        printf(1,"第%d个页的内容是: %c\n\n",i+1,adTable[i][1]);//访问第i个页
        sleep(100*3);//进程休眠，便于观察
    }
}
int
main(void)
{
    bstat();
    for(int i = 1 ; i <= 689; i++)
        sbrk(4096)[1] = 1;//把物理页分配出去，为实验做准备
    bstat();
    mem();
    return 0;
}