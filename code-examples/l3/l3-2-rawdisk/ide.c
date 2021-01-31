// Simple PIO-based (non-DMA) IDE driver code.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "fs.h"
#include "buf.h"

#define SECTOR_SIZE 512
#define IDE_BSY 0x80 //硬盘状态位1 << 7
#define IDE_DRDY 0x40 //1 << 6
#define IDE_DF 0x20   //1 << 5
#define IDE_ERR 0x01  //1 << 1

#define IDE_CMD_READ 0x20 //硬盘读命令
#define IDE_CMD_WRITE 0x30//硬盘写命令
#define IDE_CMD_RDMUL 0xc4 //??
#define IDE_CMD_WRMUL 0xc5

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;
static struct buf *idequeue;

static int havedisk1;
static int havedisk2;
static void idestart(struct buf *);

// Wait for IDE disk to become ready.
static int
idewait(int checkerr)
{
  int r;

  while (((r = inb(0x1f7)) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY) //读取硬盘状态，并等待准备好
    ;
  if (checkerr && (r & (IDE_DF | IDE_ERR)) != 0)
    return -1;
  return 0;
}

void ideinit(void)
{
  int i;

  initlock(&idelock, "ide");
  picenable(IRQ_IDE); //enable the IDE_IRQ interrupt
  ioapicenable(IRQ_IDE, ncpu - 1);
  idewait(0); //polls the status bits until the busy bit (IDE_BSY) is clear and the ready bit (IDE_DRDY) is set

  //check how many disks are present

  // Check if disk 1 is present
  outb(0x1f6, 0xe0 | (1 << 4)); //（11100000|10000）24-bit LBA方式，选择从盘
  for (i = 0; i < 1000; i++)
  { //轮训为了等待
    if (inb(0x1f7) != 0)
    { //check I/O port 0x1f7,status bits of the disk hardware
      havedisk1 = 1;
      break;
    }
  }

  outb(0x176, 0xe0 | (0 << 4)); //24-bit LBA方式，选择主盘
  for (i = 0; i < 1000; i++)
  {
    if (inb(0x177) != 0)
    {
      havedisk2 = 1;
      cprintf("have disk2\n");
      break;
    }
  }

  // Switch back to disk 0.
  outb(0x1f6, 0xe0 | (0 << 4));
}

// Start the request for b.  Caller must hold idelock.
static void
idestart(struct buf *b)
{
  if (b == 0)
    panic("idestart");
  if (b->blockno >= FSSIZE)
    panic("incorrect blockno");
  int sector_per_block = BSIZE / SECTOR_SIZE;
  int sector = b->blockno * sector_per_block;
  int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ : IDE_CMD_RDMUL;
  int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

  if (sector_per_block > 7)
    panic("idestart");

  idewait(0);
  outb(0x3f6, 0);                // generate interrupt
  outb(0x1f2, sector_per_block); // number of sectors ，扇区数
  outb(0x1f3, sector & 0xff);
  outb(0x1f4, (sector >> 8) & 0xff);
  outb(0x1f5, (sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->dev & 1) << 4) | ((sector >> 24) & 0x0f));
  if (b->flags & B_DIRTY)
  {
    outb(0x1f7, write_cmd);
    outsl(0x1f0, b->data, BSIZE / 4);
  }
  else
  {
    outb(0x1f7, read_cmd);
  }
}

// Interrupt handler.
void ideintr(void)
{
  struct buf *b;

  // First queued buffer is the active request.
  acquire(&idelock);
  if ((b = idequeue) == 0)
  {
    release(&idelock);
    // cprintf("spurious IDE interrupt\n");
    return;
  }
  idequeue = b->qnext;

  // Read data if needed.
  if (!(b->flags & B_DIRTY) && idewait(1) >= 0)
    insl(0x1f0, b->data, BSIZE / 4);

  // Wake process waiting for this buf.
  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;
  wakeup(b);

  // Start disk on next buf in queue.
  if (idequeue != 0)
    idestart(idequeue);

  release(&idelock);
}

//PAGEBREAK!
// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void iderw(struct buf *b)
{
  struct buf **pp;

  if (!(b->flags & B_BUSY))
    panic("iderw: buf not busy");
  if ((b->flags & (B_VALID | B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  if (b->dev != 0 && !havedisk1)
    panic("iderw: ide disk 1 not present");

  acquire(&idelock); //DOC:acquire-lock

  // Append b to idequeue.
  b->qnext = 0;
  for (pp = &idequeue; *pp; pp = &(*pp)->qnext) //DOC:insert-queue
    ;
  *pp = b;

  // Start disk if necessary.
  if (idequeue == b)
    idestart(b);

  // Wait for request to finish.
  while ((b->flags & (B_VALID | B_DIRTY)) != B_VALID)
  {
    sleep(b, &idelock);
  }

  release(&idelock);
}

// Read blockno block of rawdisk
/*
data寄存器	        0x1F0	已经读取或写入的数据，大小为两个字节（16位数据)每次读取1个word,反复循环，直到读完所有数据
features寄存器	    0x1F1	读取时的错误信息写入时的额外参数
sector count寄存器	0x1F2	指定读取或写入的扇区数
LBA low寄存器     	0x1F3	lba地址的低8位
LBA mid寄存器	      0x1F4	lba地址的中8位
LBA high寄存器    	0x1F5	lba地址的高8位
device寄存器	      0x1F6	lba地址的前4位（占用device寄存器的低4位）主盘值为0（占用device寄存器的第5位）第6位值为1 LBA模式为1，CHS模式为0（占用device寄存器的第7位）第8位值为1
command寄存器	      0x1F7	读取，写入的命令，返回磁盘状态1 读取扇区:0x20 写入扇区:0x30 磁盘识别:0xEC
*/
void rdraw(int blockno, char *fb)
{
  int flag = 0;
  if (blockno < 0)
  {
    flag = 1;
    blockno = -blockno - 1;
  }
  //设置ide寄存器
  while ((inb(0x177) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY)//等待硬盘准备好
    ;
  outb(0x172, 1);                      //指定读取或写入的扇区个数
  outb(0x173, blockno & 0xff);         //lba地址的低8位
  outb(0x174, (blockno >> 8) & 0xff);  //lba地址的中8位
  outb(0x175, (blockno >> 16) & 0xff); //lba地址的高8位
  outb(0x176, 0xe0 | ((0) << 4) | ((blockno >> 24) & 0x0f));//设置LBA28模式为 device0 并设置LAB高24~27位    
  //flag为真则写
  if (flag)
  {
    while ((inb(0x177) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY);//等待硬盘准备好
    outb(0x177, IDE_CMD_WRITE);//写命令
    while ((inb(0x177) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY);//等待硬盘准备好
    outsl(0x170, fb, BSIZE / 4);//io端口、内存缓存起始地址、表示输出输入的量
  }
  else
  {
    while ((inb(0x177) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY);//等待硬盘准备好
    outb(0x177, IDE_CMD_READ);//读命令
    while ((inb(0x177) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY);//等待硬盘准备好
    insl(0x170, fb, BSIZE / 4);
  }
  
}