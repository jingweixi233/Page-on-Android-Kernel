#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <asm/pgtable.h>
#include <asm/errno.h>

#define _NR_page_layout 378
#define _NR_page_table 356
#define PGD_SIZE ((1<<11)*sizeof(unsigned long))

MODULE_LICENSE("Dual BSD/GPL");

static int (*old_page_layout)(void);
static int (*old_page_table)(void);

struct pagetable_layout_info{
    uint32_t pgdir_shift;
    uint32_t page_shift;
};
struct walk_info{
    unsigned long *pgd_base;
    unsigned long  pte_base;
};
int get_pagetable_layout(struct pagetable_layout_info __user* pgtbl_info,int size){
    uint32_t pgd_sht=PGDIR_SHIFT;
    uint32_t page_sht=PAGE_SHIFT; 
    printk(KERN_INFO "get_page_table_layout invoked!\n");  
    if(pgtbl_info==NULL){
        return 0;
    }
    if(sizeof(struct pagetable_layout_info)>size){
        printk(KERN_INFO "pagetable size error!\n");
        return -1;
    }
    //copy kernel info to user
    if(copy_to_user(&(pgtbl_info->pgdir_shift),&pgd_sht,sizeof(uint32_t))){
        printk(KERN_INFO "copy pgdir_shift failed!\n");
        return -1;
    }
    if(copy_to_user(&(pgtbl_info->page_shift),&page_sht,sizeof(uint32_t))){
        printk(KERN_INFO "copy page_shift failed!\n");
        return -1;
    }
    return 0;
}
int callback_pmd_entry(pmd_t *pmd,unsigned long addr,unsigned long end,struct mm_walk *walk){
    printk("callback_pmd_entry invoked!\n");
int pfn;
    unsigned long pmdindex=(addr>>21)&0x7FF;
    if (!pmd)
    {
        printk(KERN_INFO "pmd empty!\n");
        return 0;
    }

    printk("callback_pmd_entry invoked!\n");
    struct walk_info *walk_copy=(struct walk_info*)walk->private;
    struct vm_area_struct *vma=find_vma(current->mm,walk_copy->pte_base);
    printk("find_vma invoked!\n");
    
    pfn=page_to_pfn(pmd_page((unsigned long)*pmd));
    printk("page_to _pfn invoked!\n");
    down_write(&current->mm->mmap_sem);
    remap_pfn_range(vma,walk_copy->pte_base,pfn,PAGE_SIZE,vma->vm_page_prot);
    up_write(&current->mm->mmap_sem);
    printk("remap_pfn_range invoked!\n");
    walk_copy->pgd_base[pmdindex]=walk_copy->pte_base;
    walk_copy->pte_base+=PAGE_SIZE;
    return 0;
}

int expose_page_table(pid_t pid,unsigned long fake_pgd,unsigned long page_table_addr,unsigned long begin_vaddr,unsigned long end_vaddr){
    struct pid *task_pid=NULL;
    struct task_struct *target_process=NULL;
    struct mm_struct *target_mm=NULL;
    struct vm_area_struct *target_vm_area_first=NULL;
    struct vm_area_struct *temp=NULL;
    struct walk_info walk_copy;
    struct mm_walk walk;

    printk(KERN_INFO "expose_table_page invoked!\n");
    //check virtual address space
    if(end_vaddr<=begin_vaddr){
        printk(KERN_INFO "virtual space fault!\n");
        return -1;
    }
    //get target pid struct
    task_pid=find_get_pid(pid);//get target pid_struct by using pid 
    if(task_pid==NULL){
        printk(KERN_INFO "no such pid struct!\n");
        return -1;
    }
    //get target process task_struct
    target_process=get_pid_task(task_pid,PIDTYPE_PID);//get target task_struct by using pid_struct
    if(target_process==NULL){
        printk(KERN_INFO "no such target process!\n");
        return -1;
    }
    printk(KERN_INFO "target process name:%s \n",target_process->comm);

    //visit vm_area_struct
    target_mm=target_process->mm;//pointer (*mm) in task_struct point to (mm_struct)
    target_vm_area_first=target_process->mm->mmap;//pointer (*mmap) in mm_struct point to (vm_area_struct)
    
    printk("target process virtual area:\n");
    down_write(&target_mm->mmap_sem);
    for(temp=target_vm_area_first;temp!=NULL;temp=temp->vm_next){
        printk(KERN_INFO "\t %08lx->%08lx\n",temp->vm_start,temp->vm_end);
    }
    up_write(&target_mm->mmap_sem);
    //finish finding target process virtual memory
    
    //set up callback function
    printk("==================\n");
    printk("page_table_addr:0x%08lx\n",page_table_addr);
    walk_copy.pte_base=page_table_addr;
    printk("walk_copy.pte_base:0x%08lx\n",walk_copy.pte_base);
    walk_copy.pgd_base=(unsigned long*)kmalloc(PGD_SIZE,GFP_KERNEL);
    printk("walk_copy.pgd_base:0x%08lx\n",(unsigned long)walk_copy.pgd_base);

    walk.pgd_entry 
    = NULL;//for_pgd_entry;
    walk.pud_entry = NULL;
    walk.pte_entry = NULL;
    walk.pte_hole = NULL;
    walk.hugetlb_entry = NULL;
    walk.mm=target_process->mm;
    walk.private=(void*)(&walk_copy);
    walk.pmd_entry=callback_pmd_entry;
    printk("begin_vaddr:0x%08lx \t end_vaddr:0x%08lx\n",begin_vaddr,end_vaddr);
    
    //start to walk between begin_vaddr and end_vaddr
    printk("start walk!\n");
    down_write(&target_mm->mmap_sem);
    walk_page_range(begin_vaddr,end_vaddr,&walk);
    up_write(&target_mm->mmap_sem);
    printk("end walk!\n");
    //copy to user
    if(copy_to_user((void*)fake_pgd,walk_copy.pgd_base,PGD_SIZE)){
        return -1;
    }
    kfree(walk_copy.pgd_base);
    return 0;
}
static int pagetable_init(void){
    long *syscall=(long*)0xc000d8c4;
    //save old system call and assign new system call
    old_page_layout=(int(*)(void))(syscall[_NR_page_layout]);
    syscall[_NR_page_layout]=(unsigned long)get_pagetable_layout;
    printk(KERN_INFO "get_pagetable_layout load!\n");

    //save old system call and assign new system call
    old_page_table=(int(*)(void))(syscall[_NR_page_table]);
    syscall[_NR_page_table]=(unsigned long)expose_page_table;
    printk(KERN_INFO "expose_page_table load!\n");
    return 0; 
}
static void pagetable_exit(void){
    long *syscall=(long*)0xc000d8c4;
    //reassign old system call
    syscall[_NR_page_layout]=(unsigned long)old_page_layout;
    printk(KERN_INFO "get_pagetable_layout exit!\n");
    //reassign old system call
    syscall[_NR_page_table]=(unsigned long)old_page_table;
    printk(KERN_INFO "get_pagetable_layout exit!\n");
}
module_init(pagetable_init);
module_exit(pagetable_exit);
