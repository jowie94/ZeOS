#include <libc.h>

int m = 0;

void test()
{
  write(1, "Cloned", 6);
  char ch[1];
  itoa(m, ch);
  write(1, ch, 1);
  sem_wait(0);
  write(1, "Waited", 6);
  itoa(m, ch);
  write(1, ch, 1);
  exit();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  char stack[255];
  sem_init(0, 0);
  clone(test, &stack);
write(1, "r", 1);
  yield();
  write(1, "r", 1);
  int i = 0;
  m = 1;
  //m = 1;
  sem_signal(0);
  while (1);
}
