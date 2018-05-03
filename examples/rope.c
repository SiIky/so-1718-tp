#include <stdio.h>

#include <utils/ifjmp.h>

#include "rope.h"
#include "str.h"

int usage (const char * cmd)
{
    fprintf(stderr,
            "Usage: %s FILE...\n",
            cmd);
    return !0;
}

#define BUF_SIZE 4096

int main (int argc, char ** argv)
{
    if (argc < 2)
        return usage(*argv);

    FILE * fin = NULL;
    char l[BUF_SIZE] = "";

    struct rope * r = rope_new();
    ifjmp(r == NULL, ko);

    for (int i = 1; i < argc; i++) {
        fin = fopen(argv[i], "r");
        if (fin == NULL)
            continue;

        while (fgets(l, BUF_SIZE, fin)) {
            char * ptr = calloc(BUF_SIZE, sizeof(char));
            if (ptr == NULL)
                continue;

            size_t capacity = strlen(l);
            size_t length = capacity;

            struct str * s = str_from_raw_parts(ptr, length, capacity);
            if (s == NULL) {
                free(ptr);
                continue;
            }

            strncpy(ptr, l, length);

            rope_push(r, s);
        }

        fclose(fin);

        for (rope_iter(r); rope_itering(r); rope_iter_next(r)) {
            struct str * s = rope_get_nth(r, rope_iter_idx(r));
            printf("%s", str_as_slice(s));
        }

        rope_truncate(r, 0);
    }

    rope_free(r);

    return 0;

ko:
    return !0;
}
