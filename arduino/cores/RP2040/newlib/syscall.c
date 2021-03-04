////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Georgi Angelov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <reent.h>
#include <errno.h>
#include <sys/_timeval.h>
#include <sys/stat.h>

/*
void _exit(int status) { abort(); }
int _getpid_r(struct _reent *r) { return -1; }
void _raise(void) { return; }
int __attribute__((weak)) _link(const char *__path1 __attribute__((unused)), const char *__path2 __attribute__((unused))) { return 0; }
int _unlink(const char *path){ return 0; }
*/

// abort() //////////////////////////////////////////////////////////////////////////////

void _kill(int pid, int sig) { return; }
int _getpid(void) { return -1; }

////////////////////////////////////////////////////////////////////////////////////////

int _isatty(int fd)
{
    return ((fd == STDOUT_FILENO) || (fd == STDERR_FILENO) || (fd == STDIN_FILENO));
}

_off_t _lseek_r(struct _reent *r, int fd, _off_t where, int whence)
{
#ifdef USE_FS
    return vfs_seek(fd, where, whence);
#endif
    errno = EINVAL;
    return -1;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
#ifdef USE_FS
// TODO
#endif
    errno = EINVAL;
    return -1;
}

int _close_r(struct _reent *r, int fd)
{
#ifdef USE_FS
    return vfs_close(fd);
#endif
    errno = EINVAL;
    return -1;
}

int _open_r(struct _reent *r, const char *path, int flags, int mode)
{
#ifdef USE_FS
    return vfs_open(path, flags, mode);
#endif
    errno = EINVAL;
    return -1;
}

_ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t len)
{
    if (fd < 0 || !buf || !len)
        return 0;
    if ((fd == STDOUT_FILENO) || (fd == STDERR_FILENO))
        if (stdout->_cookie && stdout->_write)
            return stdout->_write(r, stdout->_cookie, buf, len);
#ifdef USE_FS
    return vfs_write(fd, buf, len);
#endif
    return 0;
}

_ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t len)
{
    if (fd < 0 || !buf || !len)
        return 0;
    if (fd == STDIN_FILENO)
        if (stdin->_cookie && stdin->_read)
            return stdin->_read(r, stdin->_cookie, buf, len);
#ifdef USE_FS
    return vfs_read(fd, buf, len);
#endif
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

void add_syscalls(void)
{
    extern void add_time(void);
    add_time();
}