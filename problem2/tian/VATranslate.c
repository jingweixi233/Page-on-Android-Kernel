#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>
#include <malloc.h>
#include <sys/syscall.h>
#include <unistd.h>

#define _NR_page_layout 378
#define _NR_page_table 356

#define PGD_SIZE ((1<<11)*sizeof(unsigned long))
#define PTE_SIZE ((1<<20)*sizeof(unsigned long))

struct page_table_layout_info{
    uint32_t pgdir_shift;
    uint32_t page_shift;
};
int main(int argc,char **argv){
    pid_t pid;
    unsigned long begin_vaddr,end_vaddr;
    unsigned long *fake_pgd,*page_table_addr;
    /********get_pagetable_layout********/
    struct page_table_layout_info pgtbl_info;
    if(syscall(_NR_page_layout,&pgtbl_info,sizeof(struct page_table_layout_info))){
        printf("Failed to get page table layout!\n");
        return -1;
    }
    printf("==================================\n");
    printf("pagetable_layout:\n");
    printf("pgdir_shift=%u\n",pgtbl_info.pgdir_shift);
    printf("page_shift=%u\n",pgtbl_info.page_shift);
    printf("==================================\n");
    
    /********get parameter of expose_page_table********/
    if(argc!=3){
        printf("parameters error!\n");
        printf("Usage:./VATranslate pid begin_vaddr\n");
        printf("Exmaple:./VATranslate 1 8001\n");
    }
    pid=atoi(argv[1]);
    begin_vaddr=strtoul(argv[2],NULL,16);
    end_vaddr=begin_vaddr+1;
    
    //get a fake base address of pgd
    fake_pgd=malloc(2*PGD_SIZE);
    //get base address in user space the ptes mapped to
    page_table_addr=mmap(NULL,1<<12,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    printf("page_table_addr:0x%08lx\n",(unsigned long)page_table_addr);
    if(!fake_pgd||!page_table_addr){
        printf("allocate memory failed!\n");
        return -1;
    }

    /********expose_page_table********/
    unsigned long phy_addr;
    unsigned long vpgd,vpte,voffset,phy_base;
    unsigned long *pte_base;
    printf("expose invoked!\n");
    if(syscall(_NR_page_table,pid,fake_pgd,page_table_addr,begin_vaddr,end_vaddr)){
        printf("expose_page_table failed!\n");
        return -1;
    }
    printf("expose exit!\n");
    //get pgd,pte and offset 
    vpgd=(begin_vaddr>>21)&0x7FF;//pgd_index()
    vpte=(begin_vaddr>>12)&0x1FF;//pte_index()
    voffset=begin_vaddr&0xFFF;//offset
    
    pte_base=(unsigned long*)fake_pgd[vpgd];//pte_base=fake_pgd+pgd_index()
    phy_base=pte_base[vpte];//phy_base=pte_base+pte_index()
    phy_addr=phy_base+voffset;//get physical address
    
    printf("begin_address: 0x%08lx\n",begin_vaddr);
    printf("pgd: 0x%03lx\t pte: 0x%3lx\t offset: 0x%3lx\n",vpgd,vpte,voffset);
    printf("pgd_base: 0x%08lx\t pte_base: 0x%08lx\n",(unsigned long)fake_pgd,(unsigned long)pte_base);
    printf("pgd_base[0x%03lx]: 0x%08lx\t pte_base[0x%03lx]: 0x%08lx\n",vpgd,(unsigned long)pte_base,vpte,phy_addr);
    printf("physical address: 0x%08lx\n",phy_addr);

    free(fake_pgd);
    munmap(page_table_addr,1<<12);
    return 0;
}
