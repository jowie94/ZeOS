/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define next_pid() next_free_pid++

int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int ret_from_fork()
{
    return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;
  unsigned int i;

  if (list_empty(&freequeue))
    return -1;
  struct list_head *head = list_first(&freequeue);
  union task_union* child = (union task_union*) list_head_to_task_struct(head);
  union task_union* parent = (union task_union*) current();
  struct task_struct* parent_task = (struct task_struct*) parent;
  struct task_struct* child_task = (struct task_struct*) child;
  list_del(head);
  copy_data(parent, child, sizeof(union task_union));

  // Allocate new page directory for child
  allocate_DIR(&child->task);

  page_table_entry* pagt_child = get_PT(child_task);
  page_table_entry* pagt_parent = get_PT(parent_task);

  // Search and reserve free physical pages.
  int reserved_frames[NUM_PAG_DATA];
  for (i = 0; i < NUM_PAG_DATA; ++i) {
    // If not enough free frames, those reserved thus far must be freed
    if ((reserved_frames[i] = alloc_frame()) == -1) {
      while (i >= 0) free_frame(reserved_frames[i--]);
      list_add_tail(&(child_task->list), &freequeue);
      return -ENOMEM;
    }
  }

  // Inherit user data

  /* Inherit user code. As it is shared between all processes, we just need
   * to update child's page table in order to map it.
   */
  for (i = PAG_LOG_INIT_CODE; i < PAG_LOG_INIT_DATA; i++) {
      set_ss_pag(pagt_child, i, get_frame(pagt_parent, i));
  }

  unsigned int stride = PAGE_SIZE * NUM_PAG_DATA;
  for (i = 0; i < NUM_PAG_DATA; i++) {
      /* Associates a logical page from child's page table to physical reserved frame */
      set_ss_pag(pagt_child, PAG_LOG_INIT_DATA+i, reserved_frames[i]);

      /* Inherits one page of user data */
      unsigned int logic_addr = (i + PAG_LOG_INIT_DATA) * PAGE_SIZE;
      set_ss_pag(pagt_parent, i + PAG_LOG_INIT_DATA + NUM_PAG_DATA, reserved_frames[i]);
      copy_data((void *)(logic_addr), (void *)(logic_addr + stride), PAGE_SIZE);
      del_ss_pag(pagt_parent, i + PAG_LOG_INIT_DATA + NUM_PAG_DATA);
  }

  set_cr3(get_DIR(&parent->task));

  PID = next_pid();

  child_task->PID = PID;
  child_task->state = ST_READY;

  /* Prepares the return of child process. It must return 0
   * and its kernel_esp must point to the top of the stack
   */
  unsigned int ebp;
  __asm__ __volatile__(
      "mov %%ebp,%0\n"
      :"=g"(ebp)
  );

  unsigned int stack_stride = (ebp - (unsigned int)parent)/sizeof(unsigned long);

  /* Dummy value for ebp for the child process */
  child->stack[stack_stride-1] = 0;

  child->stack[stack_stride] = (unsigned long)&ret_from_fork;
  child->task.kernel_esp = &child->stack[stack_stride-1];

  /* Adds child process to ready queue and returns its PID from parent */
  list_add_tail(&(child_task->list), &readyqueue);
  return PID;
}

void sys_exit()
{
}

int sys_write(int fd, char* buffer, int size) {
  int ret = check_fd(fd, ESCRIPTURA);
  if (ret < 0) return ret;
  if (buffer == NULL || size < 0) return -9;
  char kbuff[256];
  int written = 0;
  while (size > 0) {
    int s = size < 256 ? size : 256;
    ret = copy_from_user(buffer, kbuff, s);
    if (ret == -1) return -ENOMEM;
    written += sys_write_console(kbuff, s);
    buffer+=256;
    size-=256;
  }

  return written;
}

int sys_gettime() {
  return zeos_ticks;
}
