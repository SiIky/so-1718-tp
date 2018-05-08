/*
 * <stdlib.h>
 *  free()
 *
 * <string.h>
 *  memchr()
 *  memcpy()
 *  memmove()
 */
#include <stdlib.h>
#include <string.h>

/*
 * <unistd.h>
 *  close()
 *  read()
 *  ssize_t
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/common.h>
#include <utils/ifjmp.h>
#include <utils/ifnotnull.h>
#include <tralloc.h>

#include <file.h>

struct file {
    int fd;
    size_t len;
    size_t cap;
    void * buf;
};

#define FILE_DEFAULT_CAP         1024UL
#define FILE_OFFSET(BUF, OFFSET) (((char*) (BUF)) + (OFFSET))
#define FILE_DIFF(START, END)    ((size_t) (((char*) (END)) - ((char*) (START))))

static        bool          _file_free             (struct file * self);
static        size_t        _file_fill             (struct file * self);
static        size_t        _file_read_exact_buf   (struct file * self, void * buf, size_t count);
static        size_t        _file_read_fill        (struct file * self, void * buf, size_t count);
static        size_t        _file_read_most_buf    (struct file * self, void * buf, size_t count);
static        struct file * _file_new              (void);
static        struct file * _file_with_cap         (size_t cap);
static inline size_t        _file_ssize_t_2_size_t (ssize_t n);

static bool _file_free (struct file * self)
{
    free(self->buf);
    free(self);
    return true;
}

static size_t _file_fill (struct file * self)
{
    size_t ret = 0;

    size_t free = self->cap - self->len;
    ifjmp(free == 0, out);

    ssize_t r = read(self->fd, FILE_OFFSET(self->buf, self->len), free);
    ifjmp(r <= 0, out);

    ret = (size_t) r;
    self->len += ret;

out:
    return ret;
}

/*
 * Read exactly `count` bytes from internal buffer
 * `count` has to be less than `self->len`
 */
static size_t _file_read_exact_buf (struct file * self, void * buf, size_t count)
{
    /* new internal length */
    size_t len = self->len - count;

    /* copy data from internal buffer */
    memcpy(buf, self->buf, count);

    /* update internal buffer length */
    self->len = len;

    ifjmp(len == 0, out);

    /* move data in the internal buffer to the beggining */
    memmove(self->buf, FILE_OFFSET(self->buf, count), len);

out:
    return count;
}

/*
 * try to read at most `count` to `buf`, but if `count < self->cap`
 * try to fill internal buffer
 */
static size_t _file_read_fill (struct file * self, void * buf, size_t count)
{
    size_t ret = 0;

    while (count > 0) {
        /* read from internal buffer at most count bytes */
        size_t r = _file_read_most_buf(self, buf, count);

        if (r == 0) /* could not read */
            break;

        /* increase number of read bytes */
        ret += r;

        /* advance buf by amount read */
        buf = FILE_OFFSET(buf, r);

        /* decrease wanted count */
        count -= r;

        /* fill internal buffer */
        _file_fill(self);
    }

    return ret;
}

/*
 * Read at most `count` bytes from internal buffer
 * `count` may be bigger than `self->len`
 */
static size_t _file_read_most_buf (struct file * self, void * buf, size_t count)
{
    return _file_read_exact_buf(self, buf, min(self->len, count));
}

static struct file * _file_new (void)
{
    struct file * ret = malloc(sizeof(struct file));

    if (ret != NULL) {
        ret->fd = -1;
        ret->len = 0;
        ret->cap = 0;
        ret->buf = NULL;
    }

    return ret;
}

static struct file * _file_with_cap (size_t cap)
{
    struct file * ret = _file_new();

    ifjmp(ret == NULL, out);
    ifjmp(cap == 0, out);

    ret->buf = malloc(cap);
    ret->cap = (ret->buf != NULL) ?
        cap :
        0;

out:
    return ret;
}

static inline size_t _file_ssize_t_2_size_t (ssize_t n)
{
    return (n >= 0) ? ((size_t) n) : 0;
}

bool file_close (struct file * self)
{
    if (self == NULL)
        return true;

    int fd = self->fd;
    _file_free(self);
    return close(fd) == 0;
}

int file_get_fd (const struct file * self)
{
    return (self != NULL) ? self->fd : -1;
}

size_t file_get_cap (const struct file * self)
{
    return (self != NULL) ? self->cap : 0;
}

size_t file_get_len (const struct file * self)
{
    return (self != NULL) ? self->len : 0;
}

size_t file_read (struct file * self, void * buf, size_t count)
{
    if (self == NULL || buf == NULL)
        return 0;

    if (count == 0)
        return _file_ssize_t_2_size_t(read(self->fd, NULL, 0));

    return _file_read_fill(self, buf, count);
}

/* read from internal buffer only, do not call `read()` */
size_t file_read_internal (struct file * self, void * buf, size_t count)
{
    return (self != NULL) ?
        _file_read_most_buf(self, buf, count) :
        0;
}

size_t file_readline (struct file * self, void ** buf)
{
    size_t ret = 0;

    ifjmp(buf == NULL, out);
    ifjmp(*buf != NULL, out); /* theres an addr there, dont overwrite it */
    ifjmp(self == NULL, out);

    if (self->len == 0) {
        size_t r = _file_fill(self);
        ifjmp(r == 0, out); /* prevent infinite loop */
    }

    void * ret_buf = NULL;
    size_t ret_cap = 0;
    size_t ret_len = 0;

    char * nl = NULL;
    while (nl == NULL) {
        nl = memchr(self->buf, '\n', self->len);

        /* copy the whole internal buffer if '\n' not found */
        size_t toc = (nl != NULL) ?
            FILE_DIFF(self->buf, nl) + 1 :
            self->len ;

        /* new ret_buf capacity */
        size_t nret_cap = ret_cap + toc;

        void * tmp = realloc(ret_buf, nret_cap);
        ifjmp(tmp == NULL, realloc_fail);

        ret_buf = tmp;
        ret_len += _file_read_fill(self, FILE_OFFSET(ret_buf, ret_cap), toc);
        ret_cap = nret_cap;
    }

realloc_fail:
    *buf = ret_buf;
    ret = ret_cap;

out:
    return ret;
}

#if 0
size_t file_write (struct file * self, void * buf, size_t count)
{
    size_t ret = 0;

    ifjmp(self == NULL, out);
    ifjmp(buf == NULL, out);
    ifjmp(count == 0, out);

    size_t free = self->cap - self->len;


out:
    return ret;
}
#endif

struct file * file_fdopen (int fd)
{
    struct file * ret = NULL;

    ifjmp(fd == -1, out);

    ret = _file_with_cap(FILE_DEFAULT_CAP);
    ifjmp(ret == NULL, out);

    ret->fd = fd;

out:
    return ret;
}

struct file * file_open (const char * pathname, int flags)
{
    struct file * ret = NULL;

    ifjmp(pathname == NULL, out);

    ret = _file_with_cap(FILE_DEFAULT_CAP);
    ifjmp(ret == NULL, out);

    ret->fd = open(pathname, flags);
    if (ret->fd == -1) {
        _file_free(ret);
        ret = NULL;
    }

out:
    return ret;
}

struct file * file_open_mode (const char * pathname, int flags, mode_t mode)
{
    struct file * ret = NULL;

    ifjmp(pathname == NULL, out);

    ret = _file_with_cap(FILE_DEFAULT_CAP);
    ifjmp(ret == NULL, out);

    ret->fd = open(pathname, flags, mode);
    if (ret->fd == -1) {
        _file_free(ret);
        ret = NULL;
    }

out:
    return ret;
}

struct file * file_stderr (void)
{
    return file_fdopen(STDERR_FILENO);
}

struct file * file_stdin (void)
{
    return file_fdopen(STDIN_FILENO);
}

struct file * file_stdout (void)
{
    return file_fdopen(STDOUT_FILENO);
}
