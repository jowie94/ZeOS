/*
 * libc.c
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;

  if (a==0) { b[0]='0'; b[1]=0; return ;}

  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }

  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;

  i=0;

  while (a[i]!=0) i++;

  return i;
}

void perror() {
  switch (errno) {
    case EBADF: // 9
      write(1, "Bad file number", 15);
      break;
    case ENOMEM: // 12
      write(1, "Out of memory", 13);
      break;
    case EACCES: // 13
      write(1, "Permission denied", 17);
      break;
    case ENOSYS: // 38
      write(1, "Function not implemented", 24);
      break;
    default:
      write(1, "Error", 5);
  }
}

int write(int fd, char *buffer, int size) {
  int res;
  __asm__("int $0x80"
	  : "=a" (res)
	  : "b" (fd), "c" (buffer), "d" (size), "a" (4));
  if (res < 0) {
    errno = res * -1;
    return -1;
  }
  return res;
}

int gettime() {
  int res;
  __asm__("int $0x80"
	  : "=a" (res)
	  : "a" (10));
  return res;
}
