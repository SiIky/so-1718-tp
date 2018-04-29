#ifndef _FBUF_H
#define _FBUF_H

/*
 * <stdbool.h>
 *  bool
 *
 * <stddef.h>
 *  size_t
 */
#include <stdbool.h>
#include <stddef.h>

/*
 * <fcntl.h>
 *  mode_t
 *  open()
 */
#include <fcntl.h>

struct file;

bool          file_close         (struct file * self);
int           file_get_fd        (const struct file * self);
size_t        file_get_cap       (const struct file * self);
size_t        file_get_len       (const struct file * self);
size_t        file_read          (struct file * self, void * buf, size_t count);
size_t        file_read_internal (struct file * self, void * buf, size_t count);
size_t        file_readline      (struct file * self, void ** buf);
#if 0
size_t        file_write         (struct file * self, void * buf, size_t count);
#endif
struct file * file_fdopen        (int fd);
struct file * file_open          (const char * pathname, int flags);
struct file * file_open_mode     (const char * pathname, int flags, mode_t mode);
struct file * file_stderr        (void);
struct file * file_stdin         (void);
struct file * file_stdout        (void);

#endif /* _FBUF_H */
