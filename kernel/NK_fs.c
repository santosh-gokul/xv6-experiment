#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "file.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
int
readi_nk_impl(struct inode *ip, int user_dst, uint64 dst, uint off, uint n)
{
  asm volatile("li a7, 4");
  w_a0((uint64) ip);
  w_a1(user_dst);
  w_a2(dst);
  w_a3(off);
  w_a4(n);
  asm volatile("ecall");
  return r_a0();
}

int
readi_nk(struct inode *ip, int user_dst, uint64 dst, uint off, uint n)
{

  uint tot, m;
  struct buf *bp;

  if(off > ip->size || off + n < off)
    return 0;
  if(off + n > ip->size)
    n = ip->size - off;

  for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
    uint addr = bmap(ip, off/BSIZE);
    if(addr == 0)
      break;

    bp = bread(ip->dev, addr);
    m = min(n - tot, BSIZE - off%BSIZE);
    if(either_copyout_nk(user_dst, dst, bp->data + (off % BSIZE), m) == -1) {
      brelse(bp);
      tot = -1;
      break;
    }
    brelse(bp);
  }
  return tot;
}
