#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <asm/errno.h>
#include <asm/pgtable.h>
#define __NR_get_pagetable_layout 233
#define __NR_expose_page_table 378
MODULE_LICENSE("Dual BSD/GPL");

#define PGD_SIZE ((1 << 11) * sizeof(unsigned long))
#define EACH_PTE_SIZE ((1 << 9) * sizeof(unsigned long))



struct pagetable_layout_info{
    uint32_t pgdir_shift;
    uint32_t pmd_shift;
    uint32_t page_shift;
};

struct walk_copy{
    unsigned long *fake_pgd_base;
    unsigned long fake_pmd_base;

};

static int (*oldcall_NR_get_pagetable_layout)(void), (*oldcall_NR_expose_page_table)(void);


int copy_pmd_table(pmd_t *pmd, unsigned long addr, unsigned long next, struct mm_walk *walk){
    unsigned long pgd_index = pgd_index(addr);
    struct walk_copy *copy = (struct walk_copy*)walk->private;
    int pfn, err;
    struct vm_area_struct *current_vm = find_vma(current->mm, copy->fake_pmd_base);
    int pte_size;

    pte_size = (1 << 9) * sizeof(unsigned long);

    pfn = page_to_pfn(pmd_page((unsigned long)*pmd));

    err = remap_pfn_range(current_vm, copy -> fake_pmd_base, pfn, PAGE_SIZE, current_vm -> vm_page_prot);


    copy -> fake_pgd_base[pgd_index] = copy -> fake_pmd_base;
    //printk(KERN_INFO "pgd_base[%lu] = 0x%08lx\n", pmdIndex, record->pte_base);
    copy -> fake_pmd_base += pte_size;
	return 0;
}





int get_pagetable_layout(struct pagetable_layout_info __user * pgtbl_info, int size){
	printk(KERN_INFO "start get_pagetable_layout\n");
	printk(KERN_INFO "***************************************************\n");
	struct pagetable_layout_info layout;

	layout.pgdir_shift = PGDIR_SHIFT;
	layout.pmd_shift = PMD_SHIFT;
	layout.page_shift = PAGE_SHIFT;
	
	printk(KERN_INFO "pgdir_shift:%d \n",layout.pgdir_shift);
	printk(KERN_INFO "pmd_shift:%d \n",layout.pmd_shift);
	printk(KERN_INFO "page_shift:%d \n",layout.page_shift);

	if(size < sizeof(struct pagetable_layout_info)){
		printk(KERN_INFO "size is so small\n");
        return -1;
	}
	if(copy_to_user(pgtbl_info, &layout, sizeof(struct pagetable_layout_info))){
		return -1;
	}
	printk(KERN_INFO "***************************************************\n");
	return 0;
}


int expose_page_table(pid_t pid, unsigned long fake_pgd, unsigned long page_table_addr, unsigned long begin_vaddr, unsigned long end_vaddr){
    struct pid *process_pid = NULL;
    struct task_struct *target_process = NULL;
    struct vm_area_struct *tmp;
    struct walk_copy copy;
    struct mm_walk walk = {};
    int pgd_size;
    int err;

    extern int walk_page_range(unsigned long addr, unsigned long end,
		struct mm_walk *walk);
    printk(KERN_INFO "expose_page_table\n");
    printk(KERN_INFO "***************************************************\n");
    //calculate the pgd size
    pgd_size = (1 << (32 - PGDIR_SHIFT)) * sizeof(unsigned long);
    
    process_pid = find_get_pid(pid);

    target_process = get_pid_task(process_pid, PIDTYPE_PID);
    // print target process name
    printk(KERN_INFO "process name = %s.\n", target_process->comm); 

    // travel the virtual memory areas
    down_write(&target_process->mm->mmap_sem);
    printk(KERN_INFO "Virtual memory of the target process:\n");
    for (tmp = target_process->mm->mmap; tmp; tmp = tmp->vm_next)
        printk(KERN_INFO "0x%08lx - 0x%08lx\n", tmp->vm_start, tmp->vm_end);
    up_write(&target_process->mm->mmap_sem);


    copy.fake_pgd_base = (unsigned long*)kmalloc(pgd_size, GFP_KERNEL);
    memset(copy.fake_pgd_base, 0, pgd_size);
    copy.fake_pmd_base = page_table_addr;

    walk.pgd_entry = NULL;//for_pgd_entry;
    walk.pud_entry = NULL;
    walk.pmd_entry = copy_pmd_table;//NULL;
    walk.pte_entry = NULL;
    walk.pte_hole = NULL;
    walk.hugetlb_entry = NULL;
    walk.mm = target_process -> mm;
    walk.private = (void*)(&copy);

    down_write(&target_process->mm->mmap_sem);
    walk_page_range(begin_vaddr, end_vaddr, &walk);
    up_write(&target_process->mm->mmap_sem);

    if (copy_to_user((void*)fake_pgd, copy.fake_pgd_base, pgd_size)){
        return -EFAULT;
    }
    
    kfree(copy.fake_pgd_base);
    printk(KERN_INFO "***************************************************\n");
    return 0;
}



static int pagetable_layout_init(void)
{
    long *syscall = (long*)0xc000d8c4;

    // install get_pagetable_layout()
    oldcall_NR_get_pagetable_layout = (int(*)(void))(syscall[__NR_get_pagetable_layout]); // preserve former system call
    syscall[__NR_get_pagetable_layout] = (unsigned long)get_pagetable_layout; // assign new system call
    printk(KERN_INFO "Module Get_pagetable_layout loaded successfully!\n");

    // install expose_page_table()
    oldcall_NR_expose_page_table = (int(*)(void))(syscall[__NR_expose_page_table]); // preserve former system call
    syscall[__NR_expose_page_table] = (unsigned long)expose_page_table; // assign new system call
    //printk(KERN_INFO "Module Expose_page_table loaded successfully!\n");

    return 0;
}

static void pagetable_layout_exit(void)
{
    long *syscall = (long*)0xc000d8c4;

    // uninstall get_pagetable_layout()
    syscall[__NR_get_pagetable_layout] = (unsigned long)oldcall_NR_get_pagetable_layout; // restore old system call
    printk(KERN_INFO "Module Get_pagetable_layout removed successfully!\n");

    // uninstall expose_page_table()
    syscall[__NR_expose_page_table] = (unsigned long)oldcall_NR_expose_page_table; // restore old system call
    printk(KERN_INFO "Module Expose_page_table removed successfully!\n");
}

module_init(pagetable_layout_init);
module_exit(pagetable_layout_exit);
