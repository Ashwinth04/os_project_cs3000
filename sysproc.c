#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "proc.h"

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_get_process_ancestors(void)
{
    struct ancestor_info *buffer;
    int max_entries;
    
    // Get arguments from user space
    if(argptr(0, (void*)&buffer, sizeof(struct ancestor_info)) < 0 ||
       argint(1, &max_entries) < 0)
        return -1;
        
    // Validate arguments
    if(max_entries <= 0 || buffer == 0)
        return -1;
    
    struct proc *current = myproc();
    int count = 0;
    
    acquire(&ptable.lock);
    
    while(current != 0 && count < max_entries) {
        // Safely copy data to user buffer
        if(copyout(myproc()->pgdir, 
                  (uint)&buffer[count].pid, 
                  (void*)&current->pid, 
                  sizeof(int)) < 0) {
            release(&ptable.lock);
            return -1;
        }
        
        // Set parent PID
        int ppid = (current->parent) ? current->parent->pid : -1;
        if(copyout(myproc()->pgdir, 
                  (uint)&buffer[count].parent_pid, 
                  (void*)&ppid, 
                  sizeof(int)) < 0) {
            release(&ptable.lock);
            return -1;
        }
        
        // Copy process name (optional)
        if(copyout(myproc()->pgdir, 
                  (uint)&buffer[count].name, 
                  (void*)current->name, 
                  sizeof(current->name)) < 0) {
            release(&ptable.lock);
            return -1;
        }
        
        // Move to parent
        current = current->parent;
        count++;
        
        // Stop at init process
        if(current && current->pid == 1) {
            // Include init process in the list
            if(count < max_entries) {
                if(copyout(myproc()->pgdir, 
                          (uint)&buffer[count].pid, 
                          (void*)&current->pid, 
                          sizeof(int)) < 0) {
                    release(&ptable.lock);
                    return -1;
                }
                buffer[count].parent_pid = -1;
                count++;
            }
            break;
        }
    }
    
    release(&ptable.lock);
    return count;
}
