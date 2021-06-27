#include "types.h" 
#include "stat.h" 
#include "user.h"

int main(int argc, char *argv[]) 
{ 
    if(argc <= 2) 
    { 
        printf(1, "format: chmod pathname mode\n"); 
        exit(); 
    } 
    chmod(argv[1], atoi(argv[2])); 
    exit(); 
}