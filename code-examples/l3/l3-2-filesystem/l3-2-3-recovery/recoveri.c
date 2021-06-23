#include "types.h" 
#include "stat.h" 
#include "user.h" 
#include "fcntl.h"

char buf[512];

int main(int argc, char *argv[]) 
{
    if(argc < 2) 
    { 
        printf(1, "format: savei filename temp\n"); exit();
    } 
    uint addrs[13]; 
    int fd;
    fd = open("temp", O_RDONLY); 
    read(fd, addrs, sizeof(addrs)); 
    close(fd);
    fd = open(argv[1], O_CREATE | O_RDWR); 
    int i ;
    for(i = 0; i < 13 && addrs[i] != 0; i++) {
        recoverb(addrs[0], buf);
        write(fd, buf, 512);
    } 
    close(fd); 
    exit();

}