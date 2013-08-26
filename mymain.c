/*
 *  linux/mykernel/mymain.c
 *
 *  Kernel internal my_start_kernel
 *
 *  Copyright (C) 2013  Mengning
 *
 */
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/tty.h>
#include <linux/vmalloc.h>

#include "mypcb.h"

tPCB task[MAX_TASK_NUM];
tPCB * my_current_task = NULL;
int task_count = 0;

int count = 0;
int semaphore = 0;
int ncount = 0;

int maincount = 0;
int intecount = 0;

void my_process(void);

void nonmutex(int t){
	int a = ncount;
	int c = t;
	while(c--){}
	a++;
	ncount = a;
}

int mutex(int t){
	int x=0;
	asm volatile(
		"movl $1,%%eax\n\t"
		"xchg %%eax,%1\n\t"
		"movl $0,%%ebx\n\t"
		"cmpl %%ebx,%%eax\n\t"
		"movl %%eax,%2\n\t"
		"jne 2f\n\t"
		"movl %0,%%eax\n\t"
		"movl %3,%%ecx\n\t"
		"1:\n\t"
		"nop\n\t"
		"loop 1b\n\t"
		"addl $1,%%eax\n\t"
		"movl %%eax,%0\n\t"
		"movl $0,%%eax\n\t"
		"movl %%eax,%1\n\t"
		"movl %%eax,%2\n\t"
		"2:\n\t"
		"nop\n\t"
		:"=m" (count),"=m"(semaphore),"=m"(x)
		:"m"(t)
	);
	return x;
}

void __init my_start_kernel(void)
{
    int pid = 0;
    int i;
    for(i=0;i<MAX_TASK_NUM;i++)
    {
        task[i].pid = -1;
    }
    /* Initialize process 0*/
    task[pid].pid = pid;
    task[pid].state = 0;/* -1 unrunnable, 0 runnable, >0 stopped */
    task[pid].task_entry = task[pid].thread.ip = (unsigned long)my_process;
    task[pid].thread.sp = (unsigned long)&task[pid].stack[KERNEL_STACK_SIZE-1];
    task[pid].next = &task[pid];
    /* start process 0 by task[0] */
    pid = 0;
    my_current_task = &task[pid];
    task_count = task_count + 1;
	asm volatile(
    	"movl %1,%%esp\n\t" 	/* set task[pid].thread.sp to esp */
    	"pushl %1\n\t" 	        /* push ebp */
    	"pushl %0\n\t" 	        /* push task[pid].thread.ip */
    	"ret\n\t" 	            /* pop task[pid].thread.ip to eip */
    	"popl %%ebp\n\t"
    	: 
    	: "c" (task[pid].thread.ip),"d" (task[pid].thread.sp)	/* input c or d mean %ecx/%edx*/
	);
}   

void my_process(void)
{
    while(1)
    {
	nonmutex(1000000);
	mutex(1000000);
	maincount ++;

	printk(KERN_NOTICE "this is process %d \n",my_current_task->pid);
	printk(KERN_NOTICE "count = %d , ncount = %d , diff = %d\n",count,ncount,count-ncount);
	printk(KERN_NOTICE "main=%d , inte=%d ,all=%d\n",maincount,intecount,maincount+intecount);
    }
}
