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
#include <sys/_timeval.h>
#include <sys/stat.h>

/*
void _exit(int status) { abort(); }
void _kill(int pid, int sig) { return; }
int _getpid(void) { return -1; }
int _getpid_r(struct _reent *r) { return -1; }
void _raise(void) { return; }
int __attribute__((weak)) _link(const char *__path1 __attribute__((unused)), const char *__path2 __attribute__((unused))) { return 0; }
int _unlink(const char *path){ return 0; }
*/

int _isatty(int fd)
{
    return ((fd == STDOUT_FILENO) || (fd == STDERR_FILENO) || (fd == STDIN_FILENO));
}

_off_t _lseek_r(struct _reent *r, int fd, _off_t where, int whence)
{
#ifdef USE_FS
    return vfs_seek(fd, where, whence);
#endif
    return -1;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
#ifdef USE_FS
// TODO
#endif
    return -1;
}

int _close_r(struct _reent *r, int fd)
{
#ifdef USE_FS
    return vfs_close(fd);
#endif
    return -1;
}

int _open_r(struct _reent *r, const char *path, int flags, int mode)
{
#ifdef USE_FS
    return vfs_open(path, flags, mode);
#endif
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
// TIME #include <sys/time.h>
////////////////////////////////////////////////////////////////////////////////////////

#include "interface.h"

time_t now(void)
{
    datetime_t t;
    rtc_get_datetime(&t);
    // TODO need correction
    struct tm ti;
    ti.tm_sec = t.sec;       ///< 0..59
    ti.tm_min = t.min;       ///< 0..59
    ti.tm_hour = t.hour;     ///< 0..23
    ti.tm_mday = t.day;      ///< 1..28,29,30,31 depending on month
    ti.tm_mon = t.month - 1; ///< 1..12, 1 is January
    ti.tm_year = t.year;     ///< 0..4095
    ti.tm_wday = t.dotw;     ///< 0..6, 0 is Sunday
    return mktime(&ti);
}

time_t _Time(time_t *timer) { return now(); }

time_t _time(time_t *tod)
{
    time_t t = _Time(NULL);
    if (tod)
        *tod = t;
    return (t);
}

clock_t _times(struct tms *tp)
{
    return time_us_64();
}

int _gettimeofday_r(struct _reent *ignore, struct timeval *tv, void *tz) /* time() */
{
    (void)tz;
    if (tv)
    {
        tv->tv_sec = now();
        tv->tv_usec = 0;
        return 0;
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////////////

// Executed before main ( crt0.S )
void system_init(void)
{
    // need to link boot2
    void add_bootloader(void);
    add_bootloader();

    extern void __sinit(struct _reent * s);
    __sinit(_impure_ptr);
    stdout->_cookie = 0;
    stderr->_cookie = 0;
    stdin->_cookie = 0;

    // build_flags = -D PICO_STDIO_UART
    extern void dbg_uart_init(void);
    dbg_uart_init();

    // build_flags = -D PICO_STDIO_USB
    extern void dbg_usb_init(void);
    dbg_usb_init();

    // TODO SET RTC
    datetime_t t = {
        .year = 2021,
        .month = 1,
        .day = 1,
        .dotw = 0,
        .hour = 0,
        .min = 0,
        .sec = 0};
    rtc_init(); // Start the RTC
    rtc_set_datetime(&t);
}
