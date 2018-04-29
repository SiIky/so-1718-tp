#include <stdio.h>
#include <stdlib.h>

#include "file.h"

#define FILE "test_file"

int main (void)
{
    struct file * f = file_open(FILE, O_RDONLY);

    void * buf = NULL;
    size_t r = 0;

    while ((r = file_readline(f, &buf)) > 0) {
        if (buf != NULL) {
            fwrite(buf, r, 1, stdout);
            free(buf);
            buf = NULL;
        }
    }

    file_close(f);

    return 0;
}
