#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char* argv[]){
  printf(1,"My CPU is: %d\n",getcpuid());
  exit();
}