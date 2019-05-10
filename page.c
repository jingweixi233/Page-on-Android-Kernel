#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
//#include <linux/uaccess.h>
//#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <asm/errno.h>
#include <asm/pgtable.h>

struct pagetable_layout_info{
    uint32_t pgdir_shift;
    uint32_t pmd_shift;
    uint32_t page_shift;
};

struct walk_copy{
    unsigned long *fake_pgd_base;
    unsigned long fake_pmd_base;

}

int main(){
    struct pagetable_layout_info layout;

    layout.pgdir_shift = PGDIR_SHIFT;
    layout.pmd_shift = PMD_SHIFT;
    layout.page_shift = PAGE_SHIFT;
	printf(layout.pgdir_shift);
	return 0;

}
