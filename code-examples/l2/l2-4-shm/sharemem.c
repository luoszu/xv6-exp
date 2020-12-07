#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "memlayout.h"
#define MAX_SHM_PGNUM (4) 	//每个共享内存最带4页内存

struct sharemem
{
    int refcount;     		//当前共享内存引用数，当引用数为0时才会真正回收
    int pagenum;		//占用的页数（0-4）
    void* physaddr[MAX_SHM_PGNUM];   		//对应每页的物理地址
};
struct spinlock shmlock;    			//用于互斥访问的锁
struct sharemem shmtab[8];  			//整个系统最多8个共享内存

void
sharememinit()
{
    initlock(&shmlock,"shmlock");   //初始化锁
    for (int i = 0; i < 8; i++)     //初始化shmtab
    {
        shmtab[i].refcount = 0;
    }
    
    cprintf("shm init finished.\n");
}

int
shmkeyused(uint key, uint mask)
{
    if(key<0 || 8<=key){
        return 0;
    }
    return (mask >> key) & 0x1;  //这里判断对应的系统共享内存区是否已经启用
}


int
shmrm(int key)
{
    if(key<0||8<=key){
        return -1;
    }
    //cprintf("shmrm: key is %d\n",key);
    struct sharemem* shmem = &shmtab[key];
    for(int i=0;i<shmem->pagenum;i++){
        kfree((char*)P2V(shmem->physaddr[i]));   		//逐个页帧回收
    }
    shmem->refcount = 0;
    return 0;
}


int
shmadd(uint key, uint pagenum, void* physaddr[MAX_SHM_PGNUM])
{
    if(key<0 || 8<=key || pagenum<0 || MAX_SHM_PGNUM < pagenum){
        return -1;
    }
    shmtab[key].refcount = 1;
    shmtab[key].pagenum = pagenum;
    for(int i = 0;i<pagenum;++i){
        shmtab[key].physaddr[i] = physaddr[i];
    }
    return 0;
}

int
mapshm(pde_t *pgdir, uint oldshm, uint newshm, uint sz, void **physaddr)
{
    uint a;
    if(oldshm & 0xFFF || newshm & 0xFFF || oldshm > KERNBASE || newshm < sz)
        return 0;  												//验证参数
    a=newshm;
    for (int i = 0;a<oldshm;a+=PGSIZE, i++) 					//逐页映射
    {
        mappages(pgdir,(char*)a,PGSIZE,(uint)physaddr[i],PTE_W|PTE_U);
    }
    return newshm;
}

int
deallocshm(pde_t *pgdir, uint oldshm, uint newshm)
{
	pte_t *pte;
	uint a, pa;
	if(newshm <= oldshm)
		return oldshm;
	a = (uint)PGROUNDDOWN(newshm - PGSIZE);
	for (; oldshm <= a; a-=PGSIZE)
	{
		pte = walkpgdir(pgdir,(char*)a,0);
		if(pte && (*pte & PTE_P)!=0){
			pa = PTE_ADDR(*pte);
		if(pa == 0){
			panic("kfree");
		}
		*pte = 0;
		}
	}
	return newshm;
}

// 这个方法和allcouvm实现基本一样
int
allocshm(pde_t *pgdir, uint oldshm, uint newshm, uint sz,void *phyaddr[MAX_SHM_PGNUM])
{
    char *mem;
    uint a;
    
    if(oldshm & 0xFFF || newshm & 0xFFF || oldshm > KERNBASE || newshm < sz)
        return 0;
    a = newshm;
    for (int i = 0; a < oldshm; a+=PGSIZE, i++)
    {
        mem = kalloc(); 		//分配物理页帧
        if(mem == 0){
            // cprintf("allocshm out of memory\n");
            deallocshm(pgdir,newshm,oldshm);
            return 0;
        }
        memset(mem,0,PGSIZE);
        mappages(pgdir,(char*)a,PGSIZE,(uint)V2P(mem),PTE_W|PTE_U);	//页表映射
        phyaddr[i] = (void *)V2P(mem);
        // cprintf("allocshm : %x\n",a);
    }
    return newshm;
}


void*
shmgetat(uint key, uint num)
{
    pde_t *pgdir;
    void *phyaddr[MAX_SHM_PGNUM];
    uint shm =0;
    if(key<0||8<=key||num<0||MAX_SHM_PGNUM<num) 	//校验参数
        return (void*)-1;
    acquire(&shmlock);
    pgdir = proc->pgdir;
shm = proc->shm;

// 情况1.如果当前进程已经映射了该key的共享内存，直接返回地址
    if(proc->shmkeymask>>key & 1){ 
        release(&shmlock);
        return proc->shmva[key];
}

// 情况2.如果系统还未创建此key对应的共享内存，则分配内存并映射
    if(shmtab[key].refcount == 0){
        shm = allocshm(pgdir, shm, shm - num * PGSIZE, proc->sz, phyaddr); 
//新增的allocshm()分配内存并映射，其原理和allcouvm()相同
        if(shm == 0){
            release(&shmlock);
            return (void*)-1;
        }
        proc->shmva[key] = (void*)shm;
        shmadd(key, num, phyaddr);	//将新内存区信息填入shmtab[8]数组
    }else { 

//情况3.如果未持有且已经系统中分配此key对应的共享内存，则直接映射
        for(int i = 0;i<num;i++)
        {
            phyaddr[i] = shmtab[key].physaddr[i];
        }
        num = shmtab[key].pagenum;
		//mapshm方法新建映射
        if((shm = mapshm(pgdir,shm,shm-num*PGSIZE,proc->sz,phyaddr))==0){
            release(&shmlock);
            return (void*)-1;
        }
        proc->shmva[key] = (void*)shm;
        shmtab[key].refcount++;			//引用计数+1
    }
    proc->shm = shm;
    proc->shmkeymask |= 1<<key;
    release(&shmlock);
    return (void*)shm;
}

void
shmaddcount(uint mask)
{
	acquire(&shmlock);
	for (int key = 0; key < 8; key++)
	{
	if(shmkeyused(key,mask)){   //对目前进程所有引用的共享内存的引用数加1
		shmtab[key].refcount++;   
		}
	}
	release(&shmlock);
}

int
shmrelease(pde_t *pgdir, uint shm, uint keymask)
{
    //cprintf("shmrelease: shm is %x, keymask is %x.\n",shm, keymask);
    acquire(&shmlock);
    deallocshm(pgdir,shm,KERNBASE); 				//释放用户空间的内存
    for (int k = 0; k < 8; k++)
    {
        if(shmkeyused(k,keymask)){
            shmtab[k].refcount--;   				//引用数目减1
            if(shmtab[k].refcount==0){  			//若为0 ，即可以回收物理内存
                shmrm(k);    
            }
        }
    }
    release(&shmlock);
    return 0;
}

int
shmrefcount(uint key)
{
    acquire(&shmlock);
    int count;
    count = (key<0||8<=key)? -1:shmtab[key].refcount;
    release(&shmlock);
    return count;
}

