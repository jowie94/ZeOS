#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2)
{
  __asm__ ("movl 0xc(%%ebp),%%eax\n\t"
	   "addl 0x8(%%ebp), %%eax"
	   :
	   :
	   :"%eax");
}

long inner(long n)
{
  int i;
  long suma;
  suma = 0;
  for (i = 0; i < n; ++i) suma = add(suma, i);
  return suma;
}

long outer(long n)
{
  int i;
  long acum;
  acum = 0;
  for (i = 0; i < n; ++i) acum = add(acum, inner(i));
  return acum;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  long count, acum;
  count = 75;
  acum = 0;
  //acum = outer(count);
  write(1, "Hello world!\n", 12);
  int time = gettime();
  char time_str[20];
  itoa(time, time_str);
  write(1, time_str, 20);
  int pid;
  pid = fork();
  if (pid < 0) perror();
  else if (pid == 0) write(1, "child", 5);
  else write(1, "parent", 6);
  char p[5];
  itoa(getpid(), p);
  write(1, p, 5);
  while(1) {
    itoa(getpid(), p);
    write(1, p, 5);
    write(1, "\n", 1);
    time = gettime();
    if (time > 10 && pid == 0) {
      write(1, "c exit", 6);
      exit();
    }
  }
  return 0;
}
