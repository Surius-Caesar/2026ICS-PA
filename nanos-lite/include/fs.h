#ifndef __FS_H__
#define __FS_H__

#include "common.h"

enum {SEEK_SET, SEEK_CUR, SEEK_END};

// File system interface for nanos-lite (simple ramdisk-backed FS)
int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
size_t fs_filesz(int fd);  // Get file size by fd
off_t fs_disk_offset(int fd);  // Get file offset in ramdisk
int fs_fstat(int fd, void *buf);  // Fill struct stat at buf

void init_fs(void);

#endif
