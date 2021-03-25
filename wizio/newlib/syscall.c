////////////////////////////////////////////////////////////////////////////////////////
//
//      2021 Georgi Angelov
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

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#ifdef USE_DEBUG
#include <dbg.h>
#define SYS_DBG DBG
#else
#define SYS_DBG
#endif

int vfs_open(const char *path, int flags, int ignore_mode);
int vfs_close(int fd);
size_t vfs_write(int fd, const char *buf, size_t size);
size_t vfs_read(int fd, char *buf, size_t size);
_off_t vfs_seek(int fd, _off_t where, int whence);

char *__dso_handle; //void* __dso_handle __attribute__ ((__weak__));

// abort() //////////////////////////////////////////////////////////////////////////////

void _kill(int pid, int sig) { return; }
int _getpid(void) { return -1; }

////////////////////////////////////////////////////////////////////////////////////////

int _isatty(int fd)
{
    return (unsigned int)fd <= STDERR_FILENO;
}

_off_t _lseek_r(struct _reent *r, int fd, _off_t where, int whence)
{
    int err = -EINVAL;
#ifdef USE_VFS
    err = vfs_seek(fd, where, whence);
#endif
    //__errno_r(r) = (err < 0) ? -err : 0;
    errno = (err < 0) ? -err : 0;
    return err;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
    int err = -EINVAL;
#ifdef USE_VFS
// TODO
#endif
    //__errno_r(r) = (err < 0) ? -err : 0;
    errno = (err < 0) ? -err : 0;
    return err;
}

int _close_r(struct _reent *r, int fd)
{
    int err = -EINVAL;
    if (fd > STDERR_FILENO)
    {
#ifdef USE_VFS
        err = vfs_close(fd);
#endif
    }
    //__errno_r(r) = (err < 0) ? -err : 0;
    errno = (err < 0) ? -err : 0;
    return err;
}

int _open_r(struct _reent *r, const char *path, int flags, int mode)
{
    int err = -EINVAL;
#ifdef USE_VFS
    if (path)
        err = vfs_open(path, flags, mode);
#endif
    //__errno_r(r) = (err < 0) ? -err : 0;
    errno = (err < 0) ? -err : 0;
    return err;
}

_ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t len)
{
    int err = -EINVAL;
    if (fd > -1 && buf && len)
    {
        if (_isatty(fd))
        {
#ifdef ARDUINO
            if (stdout->_cookie && stdout->_write)
                return stdout->_write(r, stdout->_cookie, buf, len);
#else // pico-sdk
#if defined(PICO_STDIO_UART) || defined(PICO_STDIO_USB) || defined(PICO_STDIO_SEMIHOST)
            //SYS_DBG("(%s) fd=%d\n", __func__, fd);
            extern int _write(int, char *, int);
            return _write(1, (char *)buf, len); // pico write to 1
#endif
#endif
        }
        else
        {
#ifdef USE_VFS
            err = vfs_write(fd, buf, len);
#endif
        }
    }
    //__errno_r(r) = (err < 0) ? -err : 0;
    errno = (err < 0) ? -err : 0;
    return err;
}

_ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t len)
{
    int err = -EINVAL;
    if (fd > -1 && buf && len)
    {
        if (fd == STDIN_FILENO)
        {
#ifdef ARDUINO
            if (stdin->_cookie && stdin->_read)
                return stdin->_read(r, stdin->_cookie, buf, len);
#else // pico-sdk
#if defined(PICO_STDIO_UART) || defined(PICO_STDIO_USB) || defined(PICO_STDIO_SEMIHOST)
            //SYS_DBG("(%s) fd=%d\n", __func__, fd);
            extern int _read(int, char *, int);
            return _read(0, buf, len); // pico read from 0
#endif
#endif
        }
        else
        {
#ifdef USE_VFS
            err = vfs_read(fd, buf, len);
#endif
        }
    }
    //__errno_r(r) = (err < 0) ? -err : 0;
    errno = (err < 0) ? -err : 0;
    return err;
}

////////////////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>

int rename(const char *src_path, const char *dst_path)
{
    int err = -EINVAL;
#ifdef USE_VFS
    //return vfs_rename(src_path, dst_path);
#endif
    //errno = EINVAL;
    return err;
}

int truncate(const char *path, off_t length)
{
    int err = -EINVAL;
#ifdef USE_VFS
    //vfs_truncate(path, length);
#endif
    //errno = EINVAL;
    return err;
}

int rmdir(const char *path)
{
    int err = -EINVAL;
#ifdef USE_VFS
    //return vfs_rmdir(path);
#endif
    //errno = EINVAL;
    return err;
}

int mkdir(const char *path, mode_t mode)
{
    int err = -EINVAL;
#ifdef USE_VFS
    //return vfs_mkdir(path, mode);
#endif
    //errno = EINVAL;
    return err;
}

////////////////////////////////////////////////////////////////////////////////////////
#include <dbg.h>

/* Executed before main ( crt0.S ) */
void system_init(void)
{
#ifdef USE_DEBUG
    DBG_INIT();
#endif

    extern void include_bootloader(void);
    include_bootloader();

    extern void include_time(void);
    include_time();

#if defined(USE_LOCK) && defined(_RETARGETABLE_LOCKING)
    extern void init_lock(void);
    init_lock();
#endif

    extern void __sinit(struct _reent *);
    __sinit(_GLOBAL_REENT); //__sinit(_impure_ptr);
    stdout->_cookie = NULL;
    stderr->_cookie = NULL;
    stdin->_cookie = NULL;

#ifdef ARDUINO

    // build_flags = -D PICO_STDIO_SEMIHOSTING
    extern void semihosting_init(void);
    semihosting_init();

    // build_flags = -D PICO_STDIO_UART
    extern void dbg_uart_init(void);
    dbg_uart_init();

    // build_flags = -D PICO_STDIO_USB
    extern void dbg_usb_init(void);
    dbg_usb_init();

    extern int SysTick_Config(uint32_t ticks);
    SysTick_Config(1 + 0xFFFFFFUL); // roll 0.134 sec

    // #define F_CPU f_cpu
    extern uint32_t f_cpu;
    f_cpu = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;

#endif
}