/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
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

  // creates the child process

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
    ret = copy_from_user(buffer, kbuff, 256);
    if (ret == -1) return -1;
    int s = size < 256 ? size : 256;
    written += sys_write_console(kbuff, s);
    buffer+=256;
    size-=256;
  }

  return written;
}
