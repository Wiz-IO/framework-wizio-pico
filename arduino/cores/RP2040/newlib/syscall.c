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

int vfs_open(const char *path, int flags, int ignore_mode);
int vfs_close(int fd);
size_t vfs_write(int fd, const char *buf, size_t size);
size_t vfs_read(int fd, char *buf, size_t size);
_off_t vfs_seek(int fd, _off_t where, int whence);

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
#ifdef USE_VFS
    return vfs_seek(fd, where, whence);
#endif
    errno = EINVAL;
    return -1;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
#ifdef USE_VFS
// TODO
#endif
    errno = EINVAL;
    return -1;
}

int _close_r(struct _reent *r, int fd)
{
    if (fd > STDERR_FILENO)
    {
#ifdef USE_VFS
        return vfs_close(fd);
#endif
    }
    errno = EINVAL;
    return -1;
}

int _open_r(struct _reent *r, const char *path, int flags, int mode)
{
#ifdef USE_VFS
    return vfs_open(path, flags, mode);
#endif
    errno = EINVAL;
    return -1;
}

_ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t len)
{
    if (fd >= STDIN_FILENO && buf && len)
    {
        if (_isatty(fd))
        {
            if (stdout->_cookie && stdout->_write)
                return stdout->_write(r, stdout->_cookie, buf, len);
        }
        else
        {
#ifdef USE_VFS
            return vfs_write(fd, buf, len);
#endif
        }
    }
    errno = EINVAL;
    return -1;
}

_ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t len)
{
    if (fd >= STDIN_FILENO && buf && len)
    {
        if (fd == STDIN_FILENO)
        {
            if (stdin->_cookie && stdin->_read)
                return stdin->_read(r, stdin->_cookie, buf, len);
        }
        else
        {
#ifdef USE_VFS
            return vfs_read(fd, buf, len);
#endif
        }
    }
    errno = EINVAL;
    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>

int rename(const char *src, const char *dst)
{
#ifdef USE_VFS
    //return vfs_rename(src, dst);
#endif
    return -1;
}

int truncate(const char *path, off_t length)
{
#ifdef USE_VFS
    //vfs_truncate(path, length);
#endif
    return -1;
}

int rmdir(const char *name)
{
#ifdef USE_VFS
    //return vfs_rmdir(name);
#endif
    return -1;
}

int mkdir(const char *name, mode_t mode)
{
#ifdef USE_VFS
    //return vfs_mkdir(name, mode);
#endif
    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////

void add_syscalls(void)
{
    extern void add_time(void);
    add_time();
}