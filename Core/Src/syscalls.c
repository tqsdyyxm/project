/**
  ******************************************************************************
  * @file    syscalls.c
  * @brief   newlib 系统调用最小实现（配合 --specs=nano.specs 使用）
  *
  * _write 转发到 Synex 串口，方便用 printf 打印调试信息；
  * 串口未初始化时自动丢弃，不影响启动流程。
  ******************************************************************************
  */

#include <sys/stat.h>
#include <stdint.h>
#include "bsp_uart.h"

/* 堆边界由链接脚本提供 */
extern char _end;

void *_sbrk(int incr)
{
    static char *heap_end = 0;
    char *prev;

    if (heap_end == 0)
    {
        heap_end = &_end;
    }
    prev = heap_end;
    heap_end += incr;
    return (void *)prev;
}

int _write(int fd, char *ptr, int len)
{
    (void)fd;
    BSP_Uart_Send((const uint8_t *)ptr, (uint32_t)len);
    return len;
}

int _read(int fd, char *ptr, int len)
{
    (void)fd;
    (void)ptr;
    (void)len;
    return 0;
}

int _close(int fd)
{
    (void)fd;
    return -1;
}

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
    return 0;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1;
}

int _getpid(void)
{
    return 1;
}

void _exit(int status)
{
    (void)status;
    while (1)
    {
    }
}
