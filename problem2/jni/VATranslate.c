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

    pid=atoi(argv[1]);
    va=strtoul(argv[2],NULL,16);
    struct pagetable_layout_info ptb_info;
    err=syscall(__NR_get_pagetable_layout, &ptb_info, sizeof(struct pagetable_layout_info));

    printf("pgdir_shift:%d \n",loinfo.pgdir_shift);
    printf("pmd_shift:%d \n",loinfo.pmd_shift);
    printf("page_shift:%d \n",loinfo.page_shift);
    

}