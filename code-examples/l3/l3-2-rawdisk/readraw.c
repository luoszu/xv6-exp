#include "types.h" 
#include "stat.h" 
#include "user.h"
int main(int argc, char *argv[])
{
    char s1[600] = "blocknum:   input wowo buf\n\0";
    char s2[600] = "blocknum:   input haha buf\n\0";
    char s3[600] = " output buf\n\0";
    int i;
    //测试写入
    for(i = -1 ; i >  -5; --i){
        printf(1,"write block %d\n",-i-1);
        s1[10] = -i-1+'0';
        rdraw(i,s1);       //写入 -6代表写入5号块
    }
    
    for(i = 0; i < 4; i++){
        printf(1,"read block %d\n",i);
        rdraw(i,s3);
        printf(1,"%s\n",&s3);
    }


    for(i = -1 ; i >  -5; --i){
        printf(1,"write block %d\n",-i-1);
        s2[10] = -i-1+'0';
        rdraw(i,s2);       //写入 -6代表写入5号块
    }

    for(i = 0; i < 4; i++){
        printf(1,"read block %d\n",i);
        rdraw(i,s3);
        printf(1,"%s\n",&s3);
    }

    exit();
}