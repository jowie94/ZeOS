/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

struct list_head freequeue;
struct list_head readyqueue;

struct task_struct *idle_task;

int next_free_pid = 2;
int curr_quantum;

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t)
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t)
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t)
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos];

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	while(1)
	{
	;
	}
}

void init_idle (void)
{
  struct list_head *first = list_first(&freequeue);
  union task_union *task = (union task_union*) list_head_to_task_struct(first);
  list_del(first);
  task->task.PID = 0;
  task->task.state = ST_READY;
  allocate_DIR(&task->task);
  unsigned long *idle_stack = task->stack;
  idle_stack[KERNEL_STACK_SIZE - 1] = (unsigned long) &cpu_idle;
  idle_stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0;
  idle_task = (struct task_struct*) task;
}

void init_task1(void)
{
  struct list_head *first = list_first(&freequeue);
  union task_union *task = (union task_union*) list_head_to_task_struct(first);
  list_del(first);
  task->task.PID = 1;
  task->task.state = ST_READY;
  set_quantum(task, DEFAULT_QUANTUM);
  allocate_DIR(&task->task);
  set_user_pages(&task->task);
  tss.esp0 = KERNEL_ESP(task);
  set_cr3(get_DIR((struct task_struct*) task));
  list_add_tail(&(task->task.list), &readyqueue);
}


void init_sched(){
  INIT_LIST_HEAD(&freequeue); // Init free queue
  int i;
  for (i = 0; i < NR_TASKS; ++i) // Initialize free queue with all tasks (empty tasks)
    list_add(&(task[i].task.list), &freequeue);
  INIT_LIST_HEAD(&readyqueue); // Init ready queue (empty)
  curr_quantum = DEFAULT_QUANTUM;
}

struct task_struct* current()
{
  int ret_value;

  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void inner_task_switch(union task_union *new)
{
  tss.esp0 = KERNEL_ESP(new);
  set_cr3(get_DIR(new));
  __asm__("movl %%ebp, %0"
          : "=g" (current()->kernel_esp));
  __asm__("movl %0, %%esp\n\t"
	  "popl %%ebp\n\t"
	  "ret"
	  : : "r" (new->task.kernel_esp));
}

void task_switch(union task_union *new)
{
  __asm__("pushl %%esi\n\t"
	  "pushl %%edi\n\t"
	  "pushl %%ebx\n\t" : : :
	  );
  inner_task_switch(new);
  __asm__("popl %%ebx\n\t"
	  "popl %%edi\n\t"
	  "popl %%esi\n\t" :);
}

int get_quantum(struct task_struct *t)
{
    return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
    t->quantum = new_quantum;
}

void update_sched_data_rr()
{
  --curr_quantum;
}

int needs_sched_rr()
{
  return curr_quantum == 0;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest)
{
  if (dest == &freequeue) t->state = ST_READY;
  else t->state = ST_BLOCKED;

  set_quantum(t, DEFAULT_QUANTUM);

  /* Remove process t from its current queue and put it to dest queue only
   * if that process is not the idle and it's not the only available whith
   * ready status.
   */
  if ((t != idle_task) & (!list_empty(&readyqueue))) {
    list_del(&(t->list));
    list_add_tail(&(t->list), dest);
  }
}

void sched_next_rr()
{
  struct task_struct* next_task;
  // If no more processes in the readyqueue set next process to idle.
  if (list_empty(&readyqueue))
    next_task = idle_task;
  else {
    next_task = list_head_to_task_struct(list_first(&readyqueue));
    next_task->state = ST_RUN;
  }
  // Sets system quantum to the next process one.
  curr_quantum = get_quantum(next_task);

  // Only switch context if next_task is not the current one.
  if (next_task != current())
    task_switch((union task_union*) next_task);
}

void schedule()
{
  update_sched_data_rr();
  if (needs_sched_rr()) {
    update_process_state_rr(current(), &readyqueue);
    sched_next_rr();
  }
}
