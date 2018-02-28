#undef __KERNEL__
#undef MODULE

#define __KERNEL__
#define MODULE

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/pid.h>

//ancestry definition as given in handout, same one found in procAncestry user function
struct ancestry{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);


asmlinkage long new_cs3013_syscall2(unsigned short *target_pid,struct ancestry *response){
	unsigned short targetid;//initalize all variables at start conforming to C90 standards
	struct task_struct * target;
	struct task_struct * permtarg;
	struct task_struct * temp;
	struct pid* targ;
	struct ancestry ancestors;
	struct ancestry * ancestorsptr = &ancestors;
	int anc = 0;
	int sib = 0;
	int chi = 0;

	if(copy_from_user(&targetid, target_pid, sizeof (unsigned short) )){//copy from user
		printk(KERN_EMERG "%d",targetid);
		printk(KERN_EMERG "Value could not be copied from user!");
		return EFAULT;//if cant copy fail out
	}

	if(copy_from_user(ancestorsptr, response, sizeof (struct ancestry) )){
		printk(KERN_EMERG "Ancestry could not be copied correctly");
		return EFAULT;//if cant copy fail out
	}
	printk(KERN_INFO "Target: %d",targetid);

	//these next lines go out to my main man Jeff Estrada for saving us with those forum hints
	//the functions were looked up in pid.h, we love you for saving us so much time and energy!
	targ = find_vpid(targetid);
	if(targ <= 0){
		printk(KERN_INFO "TARGET ID NOT RUNNING!");
		return 1;
	}
	target = pid_task(targ, PIDTYPE_PID); 
	permtarg = target;
	printk(KERN_INFO "Current Process is target process with ID: %d",target->pid);
	
	//find children of process
	printk(KERN_INFO "Process Children: ");
	list_for_each_entry(temp,&target->children,sibling){
		printk(KERN_CONT "%d ",temp->pid);
		ancestorsptr->children[chi++] = temp->pid;
		if(chi==100){
			printk(KERN_EMERG "Error too many children for data structure to hold!");
			return 1;
		}
	}
	

	//find siblings of process
	printk(KERN_INFO "Process Siblings: ");
	if(target->pid==1){
		if(copy_to_user(response, ancestorsptr, sizeof(struct ancestry))){
		printk(KERN_EMERG "Couldn't copy back to user!");
		return EFAULT;
	}
		printk(KERN_CONT "No siblings, is init process");
		printk(KERN_INFO "Process Parents: None is init ");
		return 0;
	}

	list_for_each_entry(temp,&target->parent->children,sibling){
		printk(KERN_CONT "%d ",temp->pid);
		ancestorsptr->siblings[sib++] = temp->pid;
		if(sib==100){
			printk(KERN_EMERG "Error too many siblings for data structure to hold!");
			return 1;
		}
	}


	//find parents of process
	printk(KERN_INFO "Process Parents: ");
	temp = permtarg;
	while(temp->pid!=1){
		temp = temp->parent;
		printk(KERN_CONT "%d ",temp->pid);
		ancestorsptr->ancestors[anc++] = temp->pid;
		if(anc==10){
			printk(KERN_EMERG "Error too many parents for data structure to hold!");
			return 1;
		}
	}
	
	if(copy_to_user(response, ancestorsptr, sizeof(struct ancestry))){
		printk(KERN_EMERG "Couldn't copy back to user!");
		return EFAULT;
	}
	return 0;
}

static unsigned long **find_sys_call_table(void) {
  unsigned long int offset = PAGE_OFFSET;
  unsigned long **sct;
  
  while (offset < ULLONG_MAX) {
    sct = (unsigned long **)offset;

    if (sct[__NR_close] == (unsigned long *) sys_close) {
      printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
	     (unsigned long) sct);
      return sct;
    }
    
    offset += sizeof(void *);
  }
  
  return NULL;
}

static void disable_page_protection(void) {
  /*
    Control Register 0 (cr0) governs how the CPU operates.

    Bit #16, if set, prevents the CPU from writing to memory marked as
    read only. Well, our system call table meets that description.
    But, we can simply turn off this bit in cr0 to allow us to make
    changes. We read in the current value of the register (32 or 64
    bits wide), and AND that with a value where all bits are 0 except
    the 16th bit (using a negation operation), causing the write_cr0
    value to have the 16th bit cleared (with all other bits staying
    the same. We will thus be able to write to the protected memory.

    It's good to be the kernel!
  */
  write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
  /*
   See the above description for cr0. Here, we use an OR to set the 
   16th bit to re-enable write protection on the CPU.
  */
  write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
  /* Find the system call table */
  if(!(sys_call_table = find_sys_call_table())) {
    /* Well, that didn't work. 
       Cancel the module loading step. */
    return -1;
  }
  
  /* Store a copy of all the existing functions */
  ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];


  /* Replace the existing system calls */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_cs3013_syscall2;
  
  
  enable_page_protection();
  /* And indicate the load was successful */
  printk(KERN_INFO "Loaded interceptor!");

  return 0;
}

static void __exit interceptor_end(void) {
  /* If we don't know what the syscall table is, don't bother. */
  if(!sys_call_table)
    return;
  
  /* Revert all system calls to what they were before we began. */
  disable_page_protection();
  sys_call_table[__NR_cs3013_syscall2] = (unsigned long*)ref_sys_cs3013_syscall2;
  enable_page_protection();

  printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
