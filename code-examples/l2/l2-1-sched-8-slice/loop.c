#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{       int pid;
        int data[8];
        int i,j,k;
        pid=fork();
        for(i=0;i<2;i++)
        {
            for(j=0;j<1024*100;j++)
                for(k=0;k<1024*1024;k++)
                    data[k%8]=pid*k;
         }
         printf(1,"%d ",data[0]);
         exit();
}
