#include "../include/linux/sched.h"
#include "../include/linux/mm.h"
#include "../include/asm-i386/io.h"
#include "../include/linux/rmap.h"
#include "../include/linux/list.h"
#include "../include/linux/fcntl.h"
#include "../include/linux/kernel.h"
#include "../include/linux/string.h"
#include "../include/asm/uaccess.h"
#include "../include/linux/syscalls.h"
#include "../include/asm/pgtable.h"

void
page_dump(unsigned long addr) {
  unsigned long i;
  int fd;
  mm_segment_t old_fs;

  // Generate filename to output to
  char *filename = kmalloc(32, GFP_KERNEL);
  sprintf(filename, "/tmp/%d-cp%d", current->pid, current->checkpoints);

  // Allow us to use sys_{open,write,close}
  old_fs = get_fs();
  set_fs(KERNEL_DS);
  fd = sys_open(filename, O_RDWR|O_APPEND|O_CREAT, 0777);

  if (fd) {
    // Write page header
    printk(KERN_INFO "Starting new page\n");
    char *s = kmalloc(128, GFP_KERNEL);
    sprintf(s, "-- Page at 0x%lX --\n", addr);
    sys_write(fd, s, strlen(s));
    kfree(s);

    // Loop through all addresses in page
    for (i = addr; i < addr + PAGE_SIZE; i++) {
      long int *val = (unsigned long *) i;
      char *a = kmalloc(32, GFP_KERNEL);

      // Dump address
      sprintf(a, "0x%lX ", *val);
      sys_write(fd, a, strlen(a));
      kfree(a);

      // Write newline and address at beginning of line
      if (i % 8 == 7) {
        char *newline = kmalloc(32, GFP_KERNEL);
        sprintf(newline, "\n0x%lX: ", i+1);
        sys_write(fd, newline, strlen(newline));
        kfree(newline);
      }
    }
  }

  // Cleanup
  sys_close(fd);
  set_fs(old_fs);
  kfree(filename);
}

int sys_cp_range(void *start, void *end)
{
  struct mm_struct *mm = current->active_mm;
  struct vm_area_struct *vma_start, *vma_end, *vma;
  unsigned long s = (unsigned long) start,
                e = (unsigned long) end;

  // Get start and end
  printk(KERN_INFO "Passed in <start,end> = <%lu, %lu>....\n", s, e);
  vma_start = find_vma(mm, s);
  vma_end = find_vma(mm, e);
  vma = vma_start;

  // Increase num checkpoints done
  current->checkpoints++;

  // Walk vma list
  while (vma) {
    unsigned long pgstart;

    printk(KERN_INFO "Going through new VMA <%lu, %lu>....\n",
           vma->vm_start,
           vma->vm_end);
    
    // Loop through each page
    for (pgstart = vma->vm_start; pgstart < vma->vm_end; pgstart += PAGE_SIZE) {
      // Check bounds
      if ((pgstart <= s && pgstart+PAGE_SIZE >= e)
          || (pgstart <= s && pgstart+PAGE_SIZE <= e)
          || (pgstart >= s && pgstart+PAGE_SIZE <= e)) {
        page_dump(pgstart);
      }
    }

    // Check end condition
    if (vma->vm_start == vma_end->vm_start)
      break;
    vma = vma->vm_next;
  }

  return 0;
}

int sys_inc_cp_range(void *start, void *end)
{
  struct mm_struct *mm = current->active_mm;
  struct vm_area_struct *vma_start, *vma_end, *vma;
  struct page* pg;
  pgd_t *pgd;
  pmd_t *pmd;
  pte_t *ptep, pte;

  unsigned long s = (unsigned long) start,
                e = (unsigned long) end;

  // Get start and end
  printk(KERN_INFO "Passed in <start,end> = <%lu, %lu>....\n", s, e);
  vma_start = find_vma(mm, s);
  vma_end = find_vma(mm, e);
  vma = vma_start;

  if(current->checkpoints == 0){
     printk(KERN_INFO "Incremental: calling sys_cp_range\n");
     sys_cp_range(start, end);
     return 0;
  }

  // Increase num checkpoints done
  current->checkpoints++;

  // Walk vma list
  while (vma) {
    unsigned long pgstart;

    printk(KERN_INFO "Incremental: Going through new VMA <%lu, %lu>....\n",
           vma->vm_start,
           vma->vm_end);
    
    // Loop through each page
    for (pgstart = vma->vm_start; pgstart < vma->vm_end; pgstart += PAGE_SIZE) {
      pg = follow_page(mm, pgstart, 0);
      if(!pg)
 	 printk(KERN_INFO "Page does not exist..\n");

      // Check bounds
      if(pg){
        pgd = pgd_offset(mm, pgstart);
    	pmd = pmd_offset(pgd, pgstart);
	ptep = pte_offset_map(pmd, pgstart);
	pte = *ptep;

        if(PageDirty(pg))
	   printk(KERN_INFO "Page is dirty\n");
    
    	if(pte_dirty(pte))
	   printk(KERN_INFO "pte: Page is dirty\n");

        if (((pgstart <= s && pgstart+PAGE_SIZE >= e)
             || (pgstart <= s && pgstart+PAGE_SIZE <= e)
             || (pgstart >= s && pgstart+PAGE_SIZE <= e)) && pte_dirty(pte))   {
                   page_dump(pgstart);
          }
       }
    }

    // Check end condition
    if (vma->vm_start == vma_end->vm_start)
      break;
    vma = vma->vm_next;
  }

  return 0;
}
