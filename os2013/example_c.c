#include <stdio.h>

int z = 0;
int x = 8;
int y = 8;

int add(int x,int y)
{
    return x + y;   
}

int  main()
{
	printf("x:%d,y:%d\n",x,y);
	z = add(x,y);
    printf("z:%d\n",z);
	return 0;
}
