#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "memlayout.h"

#define MAX_SHM_PGNUM (4)

struct sharemem
{
    int refcount;
    int pagenum;
    void* physaddr[MAX_SHM_PGNUM];   
};

struct spinlock shmlock;
struct sharemem shmtab[8];



void
sharememinit()
{
    initlock(&shmlock,"shmlock");
    for (int i = 0; i < 8; i++)
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
    return (mask >> key) & 0x1;
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

void
shmaddcount(uint mask)
{
    acquire(&shmlock);
    for (int key = 0; key < 8; key++)
    {
        if(shmkeyused(key,mask)){
            shmtab[key].refcount++;
        }
    }
    release(&shmlock);
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
        mem = kalloc();
        if(mem == 0){
            // cprintf("allocshm out of memory\n");
            deallocshm(pgdir,newshm,oldshm);
            return 0;
        }
        memset(mem,0,PGSIZE);
        mappages(pgdir,(char*)a,PGSIZE,(uint)V2P(mem),PTE_W|PTE_U);
        phyaddr[i] = (void *)V2P(mem);
        // cprintf("allocshm : %x\n",a);
    }
    return newshm;
}

int
copyshm(pde_t *pgdir_src, uint shm, pde_t *pgdir_dst)
{
    pte_t *pte;
    uint pa;
    for (int i = shm; i < KERNBASE; i+=PGSIZE)
    {
        if((pte = walkpgdir(pgdir_src,(void*)i,0))==0)
            panic("copyshm: pte shoule exit");
        if(!(*pte & PTE_P))
            panic("copyshm: page not present");
        pa = PTE_ADDR(*pte);
        //cprintf("copyshm\n");
        if(mappages(pgdir_dst,(void*)i, PGSIZE,pa,PTE_W|PTE_U)<0){
            deallocshm(pgdir_dst,shm,KERNBASE);
            return -1;
        }
    }
    return 0;
}

int
mapshm(pde_t *pgdir, uint oldshm, uint newshm, uint sz, void **physaddr)
{
    uint a;
    if(oldshm & 0xFFF || newshm & 0xFFF || oldshm > KERNBASE || newshm < sz)
        return 0;
    a=newshm;
    for (int i = 0;a<oldshm;a+=PGSIZE, i++)
    {
        mappages(pgdir,(char*)a,PGSIZE,(uint)physaddr[i],PTE_W|PTE_U);
    }
    return newshm;
}


void*
shmgetat(uint key, uint num)
{
    // cprintf("shmgetat: key is %d, num is %d. shm is %x\n",key, num, myproc()->shm);
    pde_t *pgdir;
    void *phyaddr[MAX_SHM_PGNUM];
    uint shm =0;
    if(key<0||8<=key||num<0||MAX_SHM_PGNUM<num)
        return (void*)-1;
    acquire(&shmlock);
    pgdir = myproc()->pgdir;
    shm = myproc()->shm;
    if(myproc()->shmkeymask>>key & 1){
        release(&shmlock);
        return myproc()->shmva[key];
    }
    if(shmtab[key].refcount == 0){
        shm = allocshm(pgdir, shm, shm - num * PGSIZE, myproc()->sz, phyaddr);
        if(shm == 0){
            release(&shmlock);
            return (void*)-1;
        }
        myproc()->shmva[key] = (void*)shm;
        shmadd(key, num, phyaddr);
    }else {
        for(int i = 0;i<num;i++)
        {
            phyaddr[i] = shmtab[key].physaddr[i];
        }
        num = shmtab[key].pagenum;
        if((shm = mapshm(pgdir,shm,shm-num*PGSIZE,myproc()->sz,phyaddr))==0){
            release(&shmlock);
            return (void*)-1;
        }
        myproc()->shmva[key] = (void*)shm;
        shmtab[key].refcount++;
    }
    myproc()->shm = shm;
    myproc()->shmkeymask |= 1<<key;
    release(&shmlock);
    return (void*)shm;
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
        kfree((char*)P2V(shmem->physaddr[i]));
    }
    shmem->refcount = 0;
    return 0;
}

int
shmrelease(pde_t *pgdir, uint shm, uint keymask)
{
    //cprintf("shmrelease: shm is %x, keymask is %x.\n",shm, keymask);
    acquire(&shmlock);
    deallocshm(pgdir,shm,KERNBASE);
    for (int k = 0; k < 8; k++)
    {
        if(shmkeyused(k,keymask)){
            shmtab[k].refcount--;
            if(shmtab[k].refcount==0){
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
