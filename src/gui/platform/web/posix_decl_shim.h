#ifndef UYA_GUI_WEB_POSIX_DECL_SHIM_H
#define UYA_GUI_WEB_POSIX_DECL_SHIM_H

#include <stddef.h>
#include <stdint.h>

/*
 * The generated Uya libc code calls a small POSIX subset directly when
 * compiling the web simulator C translation unit. Pulling in <unistd.h>
 * would also declare readlink() with the host signature, which conflicts
 * with Uya's own generated libc wrapper. Keep this shim intentionally
 * narrow so we only provide the declarations that are otherwise missing.
 */
int64_t write(int32_t fd, const char *buf, size_t count);
int64_t read(int32_t fd, char *buf, size_t count);
int close(int fd);
int64_t lseek(int fd, int64_t offset, int whence);
int access(const char *pathname, int mode);
void _exit(int status);

#endif
