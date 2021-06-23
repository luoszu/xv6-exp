#include "types.h" 
#include "stat.h" 
#include "user.h" 
#include "fcntl.h"


int main(int argc, char *argv[]) {

    if(argc < 2) { 
        printf(1, "format: savei filename temp\n"); 
        exit(); 
    }
    uint addrs[13]; 
    geti(argv[1], addrs);
    int fd = open("temp", O_CREATE | O_RDWR); 
    write(fd, addrs, sizeof(addrs)); 
    close(fd); 
    exit();
}