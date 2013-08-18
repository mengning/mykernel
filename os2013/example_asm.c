#include <stdio.h>

int z = 0;
int x = 8;
int y = 8;

int  main()
{
	printf("x:%d,y:%d\n",x,y);
	/* simu z = add(x,y); */
	asm volatile(
        /* simu call */
		"pushl %1\n\t"	
		"pushl %2\n\t"
        "pushl $0\n\t"
        /* simu add functon */	
        "pushl %%ebp\n\t"
        "movl %%esp,%%ebp\n\t"
        "movl $0,%%eax\n\t"
        "addl 12(%%ebp),%%eax\n\t"
        "addl 8(%%ebp),%%eax\n\t"
        "movl %%ebp,%%esp\n\t"
        "popl %%ebp\n\t"
        /* add functon end */
        "movl %%eax,%0\n\t"
        :"=m"(z)
		: "m" (x),"m" (y)	
    );
    printf("z:%d\n",z);
	return 0;
}
