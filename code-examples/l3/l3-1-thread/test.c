#include "fcntl.h"
#include "types.h"
#include "user.h"
volatile int global = 1;
int F(int n) {
  if (n < 0)
    printf(1, "input a positive integer\n");
  else if (n == 1 || n == 2)
    return 1;
  else {
    return F(n - 1) + F(n - 2);
  }
  return 0;
}
void worker(void* arg) {
    printf(1, "thread %d is worker.\n", *(int*)arg);
    global = F(15);
    write(3, "hello\n", 6);
    exit();
}

int main(){
    int t = 1;
    open("tmp", O_RDWR | O_CREATE);
    int pid = thread_create(worker, &t);
    thread_join();
    printf(1, "thread id = %d\n", pid);
    printf(1, "global = %d\n", global);
    exit();
}
