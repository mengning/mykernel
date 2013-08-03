/*
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_start_kernel
 *
 *  Copyright (C) 2013  Mengning
 *
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

#ifdef CONFIG_X86_LOCAL_APIC
#include <asm/smp.h>
#endif

#include "mypcb.h"
char stack[MAX_TASK_NUM][KERNEL_STACK_SIZE];
tPCB task[MAX_TASK_NUM];
tPCB * my_current_task;

void my_cpu_idle(void);
void process1(void);
void process2(void);
void process3(void);

void __init my_start_kernel(void)
{
    int pid = 3;
    /* Initialize process 3*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].stack = stack[task[pid].pid + 1];
    task[pid].thread.ip = (unsigned long)process1;
    task[pid].thread.esp = (unsigned long)task[pid].stack;
    task[pid].thread.ebp = (unsigned long)task[pid].stack;
    task[pid].next = &task[0];;
    pid = 2;
    /* Initialize process 2*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].stack = stack[task[pid].pid + 1];
    task[pid].thread.ip = (unsigned long)process1;
    task[pid].thread.esp = (unsigned long)task[pid].stack;
    task[pid].thread.ebp = (unsigned long)task[pid].stack;
    task[pid].next = &task[pid+1];
    pid = 1;
    /* Initialize process 1*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].stack = stack[task[pid].pid + 1];
    task[pid].thread.ip = (unsigned long)process1;
    task[pid].thread.esp = (unsigned long)task[pid].stack;
    task[pid].thread.ebp = (unsigned long)task[pid].stack;
    task[pid].next = &task[pid+1];
    pid = 0;
    /* Initialize process 0*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].stack = stack[task[pid].pid + 1];
    task[pid].thread.ip = (unsigned long)my_cpu_idle;
    task[pid].thread.esp = (unsigned long)task[pid].stack;
    task[pid].thread.ebp = (unsigned long)task[pid].stack;
    task[pid].next = &task[pid+1];
    /* start process 0 by task[0] */
    pid = 0;
    my_current_task = &task[pid];
	asm volatile(
	"movl $0,%%eax\n\t" 	/* clear %eax to 0*/
	"movl %1,%%esp\n\t" 	/* set task[pid].thread.esp to esp */
	"movl %1,%%ebp\n\t" 	/* set to task[pid].thread.ebp ebp */
	"pushl %0\n\t" 	        /* push task[pid].thread.ip */
	"ret\n\t" 	    /* pop task[pid].thread.ip to eip */
	: 
	: "c" (task[pid].thread.ip),"d" (task[pid].thread.esp)	/* input c or d mean %ecx/%edx*/
	);
}   
void my_cpu_idle(void)
{
    int i = 0;
    while(1)
    {
        i++;
        if(i%100000 == 0)
            printk(KERN_NOTICE "my_cpu_idle  %d \n",i);      
    }
}
void process1(void)
{
    int i = 0;
    while(1)
    {
        i++;
        if(i%100000 == 0)
            printk(KERN_NOTICE "process11111111111\n");     
    }    
}
void process2(void)
{
    int i = 0;
    while(1)
    {
        i++;
        if(i%100000 == 0)
            printk(KERN_NOTICE "process2222222222\n");     
    } 
}
void process3(void)
{
    int i = 0;
    while(1)
    {
        i++;
        if(i%100000 == 0)
            printk(KERN_NOTICE "process333333333\n");     
    } 
}