#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

int _close_r(struct _reent* r, int fd)
{
	r->_errno = EBADF;
	return -1;
}
int _execve_r(struct _reent* r, const char * path, char *const argv[], char *const envp[])
{
	r->_errno = EINVAL;
	return -1;
}
int _fcntl_r(struct _reent* r, int fd, int cmd, int arg)
{

}
int _fork_r(struct _reent* r)
{

}
int _fstat_r(struct _reent* r, int fd, struct stat* buf)
{

}
int _getpid_r(struct _reent* r)
{

}
int _isatty_r(struct _reent* r, int fd)
{

}
int _kill_r(struct _reent* r, int pid, int sig)
{

}
int _link_r(struct _reent* r, const char* path1, const char* path2)
{

}
_off_t _lseek_r(struct _reent* r, int fd, _off_t offset, int whence)
{

}
int _mkdir_r(struct _reent* r, const char* path, int mode)
{

}
int _open_r(struct _reent* r, const char* path, int oflag, int mode)
{

}
_ssize_t _read_r(struct _reent* r, int, void* buf, size_t cnt)
{

}
int _rename_r(struct _reent* r, const char *, const char *)
{

}
void *_sbrk_r(struct _reent* r, ptrdiff_t incr)
{

}
int _stat_r(struct _reent* r, const char* file, struct stat* pstat)
{

}
_CLOCK_T_ _times_r(struct _reent* r, struct tms* buf)
{

}
int _unlink_r(struct _reent* r, const char* file)
{

}
int _wait_r(struct _reent* r, int* status)
{

}
_ssize_t _write_r(struct _reent* r, int fd, const void* buf, size_t cnt)
{

}

/* This one is not guaranteed to be available on all targets.  */
int _gettimeofday_r(struct _reent* r, struct timeval *__tp, void *__tzp)
{

}