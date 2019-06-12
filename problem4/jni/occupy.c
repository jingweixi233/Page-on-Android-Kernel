/*
 * This prpgram tries to occupy up to 1GB memory
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>


int main()
{
    unsigned long **p;
    unsigned long i, j;
	printf("Occupy 1GB memory.\n");
    p = (unsigned long**)malloc(1 << 15);

    
        for (i = 0; i < (1 << 13); ++i)
        {
        	p[i] = (unsigned long*)malloc(1 << 17);
        	for (j = 0; j < 32; ++j)
        		p[i][j << 10] = 0;
        }
sleep(1);

    if (p)
    	free(p);
	return 0;
}
