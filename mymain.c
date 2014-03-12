/*	mykernel--Simple simulation of the linux OS  process schedule
 *
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_timer_handler
 *
 *  Copyright (C) 2013  Mengning
 *  
 *  Modified zhyq
 * 
 * 
 *  You can redistribute or modify this program under the terms
 *  of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 

#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/stackprotector.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/bootmem.h>
#include <linux/acpi.h>
#include <linux/tty.h>
#include <linux/percpu.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/kernel_stat.h>
#include <linux/start_kernel.h>
#include <linux/security.h>
#include <linux/smp.h>
#include <linux/profile.h>
#include <linux/rcupdate.h>
#include <linux/moduleparam.h>
#include <linux/kallsyms.h>
#include <linux/writeback.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/cgroup.h>
#include <linux/efi.h>
#include <linux/tick.h>
#include <linux/interrupt.h>
#include <linux/taskstats_kern.h>
#include <linux/delayacct.h>
#include <linux/unistd.h>
#include <linux/rmap.h>
#include <linux/mempolicy.h>
#include <linux/key.h>
#include <linux/buffer_head.h>
#include <linux/page_cgroup.h>
#include <linux/debug_locks.h>
#include <linux/debugobjects.h>
#include <linux/lockdep.h>
#include <linux/kmemleak.h>
#include <linux/pid_namespace.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/idr.h>
#include <linux/kgdb.h>
#include <linux/ftrace.h>
#include <linux/async.h>
#include <linux/kmemcheck.h>
#include <linux/sfi.h>
#include <linux/shmem_fs.h>
#include <linux/slab.h>
#include <linux/perf_event.h>
#include <linux/file.h>
#include <linux/ptrace.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>

#include <asm/io.h>
#include <asm/bugs.h>
#include <asm/setup.h>
#include <asm/sections.h>
#include <asm/cacheflush.h>

#include <linux/vmalloc.h>

#include <linux/module.h> 
#include <linux/kernel.h>

#ifdef CONFIG_X86_LOCAL_APIC
#include <asm/smp.h>
#endif
#include "mypcb.h"

tPCB task[MAX_TASK_NUM];
tPCB * my_current_task = NULL;
volatile int my_need_sched = 0;

void my_process(void);
unsigned long get_rand(int );

void sand_priority(void)
{
	int i;
	for(i=0;i<MAX_TASK_NUM;i++)
		task[i].priority=get_rand(PRIORITY_MAX);
}
void __init my_start_kernel(void)
{
    int pid = 0;
    /* Initialize process 0*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    // set task 0 execute entry address to my_process
    task[pid].task_entry = task[pid].thread.ip = (unsigned long)my_process;
    task[pid].thread.sp = (unsigned long)&task[pid].stack[KERNEL_STACK_SIZE-1];
    task[pid].next = &task[pid];
    /*fork more process */
    for(pid=1;pid<MAX_TASK_NUM;pid++)
    {
        memcpy(&task[pid],&task[0],sizeof(tPCB));
        task[pid].pid = pid;
        task[pid].state = -1;
        task[pid].thread.sp = (unsigned long)&task[pid].stack[KERNEL_STACK_SIZE-1];
	task[pid].priority=get_rand(PRIORITY_MAX);//each time all tasks get a random priority
    }
	task[MAX_TASK_NUM-1].next=&task[0];
    printk(KERN_NOTICE "\n\n\n\n\n\n                system begin :>>>process 0 running!!!<<<\n\n");
    /* start process 0 by task[0] */
    pid = 0;
    my_current_task = &task[pid];
asm volatile(
     "movl %1,%%esp\n\t" /* set task[pid].thread.sp to esp */
     "pushl %1\n\t" /* push ebp */
     "pushl %0\n\t" /* push task[pid].thread.ip */
     "ret\n\t" /* pop task[pid].thread.ip to eip */
     "popl %%ebp\n\t"
     :
     : "c" (task[pid].thread.ip),"d" (task[pid].thread.sp)	/* input c or d mean %ecx/%edx*/
);
}
void my_process(void)
{
    int i = 0;
    while(1)
    {
        i++;
        if(i%10000000 == 0)
        {
	  
            if(my_need_sched == 1)
            {
                my_need_sched = 0;
		sand_priority();
	   	my_schedule();  
		
	   }
        }
    }
}//end of my_process

//produce a random priority to a task
unsigned long get_rand(max)
{
	unsigned long a;
	unsigned long umax;
	umax=(unsigned long)max;
 	get_random_bytes(&a, sizeof(unsigned long ));
	a=(a+umax)%umax;
	return a;
}
