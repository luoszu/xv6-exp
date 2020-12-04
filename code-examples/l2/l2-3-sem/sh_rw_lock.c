#include "types.h"
#include "stat.h"
#include "user.h"

int main(){
	int id=sem_create(1);
	int pid = fork();
	int i;
	for(i=0;i<100000;i++){
		sem_p(id);
		sh_var_write(sh_var_read()+1);
		sem_v(id);
	}
	if(pid >0){
		wait();
		sem_free(id);
	}
	printf(1,"sum=%d\n",sh_var_read());
	exit();
}
