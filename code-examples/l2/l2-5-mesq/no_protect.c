#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char const *argv[])
{
    char *p = (char *)0x0740;
    for (int i = 0x0000; i < 0x08; i++)
    {
        *(p+i) = '*';
    }
    
    printf(1,"This string shouldn't be modified!\n");
    exit();
}
