#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#define __NR_get_pagetable_layout 233
#define __NR_expose_page_table 378

struct pagetable_layout_info{
    uint32_t pgdir_shift;
    uint32_t pmd_shift;
    uint32_t page_shift;
};


int main(int argc, char *argv[]){
	unsigned long va;
	pid_t pid;
	int err = 0;
	unsigned long *fake_pgd_addr;
	unsigned long *fake_pmd_vaddr;
	unsigned long page_size;
	unsigned long pgd_size;
	//unsigned long
	
	unsigned long pgd_index;
	unsigned long pmd_index;

	struct pagetable_layout_info ptb_info;


	//page_table_layout_info

	//pid=atoi(argv[1]);
	//va=strtoul(argv[2],NULL,16);

	pid = 1;
	va = 8001;
	
	
	err=syscall(__NR_get_pagetable_layout, &ptb_info, sizeof(struct pagetable_layout_info));

	printf("pgdir_shift:%d \n",ptb_info.pgdir_shift);
	printf("pmd_shift:%d \n",ptb_info.pmd_shift);
	printf("page_shift:%d \n",ptb_info.page_shift);


	//expose_page_table
	
	page_size =  1<<(ptb_info.page_shift);
	pgd_size = 1<<(32 - ptb_info.pgdir_shift);

	printf("pagesize = %d\n", page_size);
	printf("pgdsize = %d\n", sizeof(unsigned long)*page_size);
	
	fake_pmd_vaddr = mmap(NULL,page_size,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fake_pgd_addr = malloc(sizeof(unsigned long) * pgd_size);

	printf("fake_pmd_vaddr = %p\n", fake_pmd_vaddr);
	printf("fake_pgd_addr = %p\n", fake_pgd_addr);

	
	
	//err = syscall(__NR_expose_page_table, pid, fake_pgd_address, table_addr, va, va+1);
	
	if(err == 0){
		printf("syscall");
	}
	


    

}
