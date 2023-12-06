struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct logging_params;
struct superblock;

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);
void            bpin(struct buf*);
void            bunpin(struct buf*);

// console.c
void            consoleinit(void);
void            consoleintr(int);
void            consputc(int);

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, uint64, int n);
int             filestat(struct file*, uint64 addr);
int             filewrite(struct file*, uint64, int n);

// fs.c
void            fsinit(int);
int             dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit();
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, int, uint64, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, int, uint64, uint, uint);
void            itrunc(struct inode*);
uint            bmap(struct inode *, uint);

// ramdisk.c
void            ramdiskinit(void);
void            ramdiskintr(void);
void            ramdiskrw(struct buf*);

// kalloc.c
void*           kalloc(void);
void            kfree(void *);
void            kinit(void);

// NK_kalloc.c
void*           kalloc_nk(void);
void            kfree_nk(void *);
void            kinit_nk(void);

// log.c
void            initlog(int, struct superblock*);
void            log_write(struct buf*);
void            begin_op(void);
void            end_op(void);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, uint64, int);
int             pipewrite(struct pipe*, uint64, int);

// printf.c
void            printf(char*, ...);
void            panic(char*) __attribute__((noreturn));
void            printfinit(void);

// proc.c
int             cpuid(void);
void            exit(int);
int             fork(void);
int             growproc(int);
void            proc_mapstacks(pagetable_t);
pagetable_t     proc_pagetable(struct proc *);
void            proc_freepagetable(pagetable_t, uint64);
int             kill(int);
int             killed(struct proc*);
void            setkilled(struct proc*);
struct cpu*     mycpu(void);
struct cpu*     getmycpu(void);
struct proc*    myproc();
void            procinit(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(uint64);
void            wakeup(void*);
void            yield(void);
int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int             either_copyin(void *dst, int user_src, uint64 src, uint64 len);
void            procdump(void);
void            forkret_NK(void);

// swtch.S
void            swtch(struct context*, struct context*);

// spinlock.c
void            acquire(struct spinlock*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            push_off(void);
void            pop_off(void);

// sleeplock.c
void            acquiresleep(struct sleeplock*);
void            releasesleep(struct sleeplock*);
int             holdingsleep(struct sleeplock*);
void            initsleeplock(struct sleeplock*, char*);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// syscall.c
void            argint(int, int*);
int             argstr(int, char*, int);
void            argaddr(int, uint64 *);
int             fetchstr(uint64, char*, int);
int             fetchaddr(uint64, uint64*);
void            syscall();

// trap.c
extern uint     ticks;
void            trapinit(void);
void            trapinithart(void);
extern struct spinlock tickslock;
void            usertrapret(void);
void            nkerneltrap(void);
// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);
void            uartputc_sync(int);
int             uartgetc(void);

// vm.c
void            kvminit(void);
void            kvminithart(void);
void            kvmmap(pagetable_t, uint64, uint64, uint64, int);
int             mappages(pagetable_t, uint64, uint64, uint64, int);
pagetable_t     uvmcreate(void);
void            uvmfirst(pagetable_t, uchar *, uint);
uint64          uvmalloc(pagetable_t, uint64, uint64, int);
uint64          uvmdealloc(pagetable_t, uint64, uint64);
int             uvmcopy(pagetable_t, pagetable_t, uint64);
void            uvmfree(pagetable_t, uint64);
void            uvmunmap(pagetable_t, uint64, uint64, int);
void            uvmclear(pagetable_t, uint64);
pte_t *         walk(pagetable_t, uint64, int);
uint64          walkaddr(pagetable_t, uint64);
int             copyout(pagetable_t, uint64, char *, uint64);
int             copyin(pagetable_t, char *, uint64, uint64);
int             copyinstr(pagetable_t, char *, uint64, uint64);

// plic.c
void            plicinit(void);
void            plicinithart(void);
int             plic_claim(void);
void            plic_complete(int);

// virtio_disk.c
void            virtio_disk_init(void);
void            virtio_disk_rw(struct buf *, int);
void            virtio_disk_intr(void);


//NK_proc.c
void            yield_nk(void);
void            sched_nk(void);
pagetable_t     proc_pagetable_nk(struct proc *);
pagetable_t     proc_pagetable_nk_impl(struct proc *);
void            forkret_NK(void);
void            forkret_NK_impl(void);
void            usertrapret_NK(void);
int             either_copyout_nk(int , uint64 , void *, uint64);
int             either_copyout_nk_impl(int , uint64 , void *, uint64);
void            freeproc_nk(struct proc*);
void            freeproc_nk_impl(struct proc*);
void            reparent_nk(struct proc* p);
void            reparent_nk_impl(struct proc* p);
void            set_protected_proc_state(struct proc*, enum procstate);
void            set_protected_proc_channel(struct proc*, void *);
void            set_protected_proc_sz(struct proc* , uint64 );
void            set_protected_proc_pagetable(struct proc* , pagetable_t);
void            set_protected_proc_ofile(struct proc*, int, struct file *);
void            swtch_nk_call(struct context *, struct context *);
void            set_protected_proc_trapframe(struct proc*, uint64, uint64);


//NK_exec.c

void            exec_nk_impl(char *, char **);
void            exec_nk(char *, char **);

//NK_vm.c
void            kvminit_nk(void);
void            kvminithart_nk(void);
void            kvmmap_nk(pagetable_t, uint64, uint64, uint64, int);
int             mappages_nk(pagetable_t, uint64, uint64, uint64, int);
pagetable_t     uvmcreate_nk(void);
uint64          uvmalloc_nk(pagetable_t, uint64, uint64, int);
uint64          uvmalloc_nk_impl(pagetable_t, uint64, uint64, int);
//uint64          uvmdealloc(pagetable_t, uint64, uint64);
void            uvmfree_nk_impl(pagetable_t, uint64);
void            uvmunmap_nk_impl(pagetable_t, uint64, uint64, int);
void            uvmclear_nk(pagetable_t, uint64);
void            uvmclear_nk_impl(pagetable_t, uint64);
pte_t *         walk_nk(pagetable_t, uint64, int);
uint64          walkaddr_nk(pagetable_t, uint64);
uint64          walkaddr_nk_impl(pagetable_t, uint64);
int             copyout_nk(pagetable_t, uint64, char *, uint64);
int             copyout_nk_impl(pagetable_t, uint64, char *, uint64);
void            proc_freepagetable_nk_impl(pagetable_t, uint64);

//NK_trap.c
void            usertrapret_NK_impl(void);
void            syscall_handler(void);
void            init_protected_proc(struct proc*);

//NK_fs.c
int             readi_nk_impl(struct inode *, int, uint64, uint, uint);
int             readi_nk(struct inode *, int, uint64, uint, uint);

//NK_spinlock.c
void            acquire_nk(struct spinlock *);
void            release_nk(struct spinlock *);

//NK_log.c
void            log_syscalls(struct logging_params *);
void            fetch_syscall_logs(struct logging_params**);
// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
