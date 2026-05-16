#ifndef __FS_H__
#define __FS_H__

#include "common.h"

enum {SEEK_SET, SEEK_CUR, SEEK_END};

// File system interface for nanos-lite (simple ramdisk-backed FS)
int fs_open(const char *pathname, int flags, int mode);
int fs_read(int fd, void *buf, size_t len);
int fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

void init_fs(void);

#endif
