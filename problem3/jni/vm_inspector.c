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
	unsigned long va_num;
	unsigned long begin_va;
    unsigned long end_va;
	pid_t pid;
	int err = 0;

	unsigned long *fake_pgd_addr;
	unsigned long *fake_pmd_addr;
	unsigned long page_size;
	unsigned long pgd_size;
	unsigned long pmd_space_size;
	
	unsigned long pgd_index;
	unsigned long pmd_index;
	unsigned long offset;

	unsigned long *pmd_base;
	unsigned long page_frame;
	unsigned long physical_address;

    unsigned long begin_va_num;
    unsigned long end_va_num;

	struct pagetable_layout_info ptb_info;

	//page_table_layout_info

	pid=atoi(argv[1]);
	begin_va=strtoul(argv[2],NULL, 16);
    end_va = strtoul(argv[3], NULL, 16);
    
	printf("virtual address = 0x%08lx\n", begin_va);
	printf("virtual address = 0x%08lx\n", end_va);

	err=syscall(__NR_get_pagetable_layout, &ptb_info, sizeof(struct pagetable_layout_info));


	//expose_page_table
	
	page_size =  1<<(ptb_info.page_shift);
	pgd_size = 1<<(32 - ptb_info.pgdir_shift) *  sizeof(unsigned long);
	pmd_space_size = pgd_size * (1 << 9);

	fake_pmd_addr = mmap(NULL, pmd_space_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fake_pgd_addr = malloc(pgd_size);

	err = syscall(__NR_expose_page_table, pid, fake_pgd_addr, fake_pmd_addr, begin_va, begin_va + 1);
	
	if(err != 0){
		return -1;
	}

	//calculate the physical address;

    begin_va_num = begin_va >> (ptb_info.page_shift);
    end_va_num = end_va >> (ptb_info.page_shift);

    printf("vm_inspector\n");
    printf("Page number\t\t\tFrame number\n");

	
/*
    for(va_num = begin_va_num; va_num < end_va_num; va_num++){

        pgd_index = (va_num >> 9) & 0x7FF;
        pmd_index = va_num & 0x1FF;

        pmd_base = (unsigned long*)((unsigned long*)fake_pgd_addr)[pgd_index];
        if(pmd_base){
            page_frame = (pmd_base[pmd_index] & 0xFFFFF000 ) >> (ptb_info.page_shift);
            if(page_frame){
                printf("0x%08lx\t\t\t0x%08lx\n", va_num, page_frame);
            }
        }
    }

*/
	free(fake_pgd_addr);
	munmap(fake_pmd_addr, pmd_space_size);
	return 0;
}
