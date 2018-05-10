#include <ctype.h>

#include <unistd.h>

#include <utils/ifjmp.h>
#include <utils/ifnotnull.h>

#include <file.h>
#include <outputs.h>
#include <rope.h>
#include <str.h>
#include <tralloc.h>
#include <outputs.h>

#define stdinfd  0
#define stdoutfd 1

int usage (const char * cmd)
{
    (void) cmd;
    return !0;
}

#define skipws(l)                                                            \
    do {                                                                     \
        while (str_itering(l) && isspace(str_get_nth(l, str_iter_idx(l)))) { \
            str_iter_next(l);                                                \
        }                                                                    \
    } while (0)

#define skipnws(l)                                                            \
    do {                                                                      \
        while (str_itering(l) && !isspace(str_get_nth(l, str_iter_idx(l)))) { \
            str_iter_next(l);                                                 \
        }                                                                     \
    } while (0)

struct rope * words (struct str * l)
{
    struct rope * ret = rope_new();
    ifjmp(ret == NULL, out);

    str_iter(l);
    skipnws(l);
    skipws(l);

    while (str_itering(l)) {
        size_t i = str_iter_idx(l);

        skipnws(l);

        size_t f = (str_itering(l)) ?
            str_iter_idx(l) - 1:
            str_iter_idx(l);

        {
            struct str * w = str_with_capacity(f - i + 1);
            ifjmp(w == NULL, out);
            for (size_t c = i; c <= f; c++)
                str_push(w, str_get_nth(l, c));
            str_push(w, '\0');

            rope_push(ret, w);
        }

        skipws(l);
    }

out:
    return ret;
}
#undef skipws
#undef skipnws

char ** line_to_argv (struct str * line)
{
    struct rope * r = words(line);
    size_t wc = rope_len(r);
    char ** ret = trcalloc(wc + 1, sizeof(char*));
    ifjmp(ret == NULL, out);

    for (size_t i = 0; i < wc; i++) {
        struct str * w = rope_remove(r, 0);
        ret[i] = str_as_mut_slice(w);
        str_set_len(w, 0);
        trfree(w);
    }

    ret[wc] = NULL;

out:
    return ret;
}

#define is_pipe(line) ((str_get_nth((line), 1) == '|') || isdigit(str_get_nth((line), 1)))

void process_cmd_line (struct rope * rope, struct str * line, struct ovec * outputs)
{
    /* merdas comuns */
    bool ip = is_pipe(line);
    char ** argv = NULL;
    int inpipe[2];
    int outpipe[2];
    struct file * outf = NULL;

    pipe(outpipe);

    if (ip) /* merdas pro pipe */
        pipe(inpipe);

    argv = line_to_argv(line);
    ifjmp(argv == NULL, cleanup);

    /* executar */
    pid_t c = fork();
    ifjmp(c == -1, cleanup);

    if (c == 0) {
        dup2(outpipe[1], stdoutfd);

        if (ip)
            dup2(inpipe[0], stdinfd);

        close(outpipe[0]);
        close(outpipe[1]);
        close(inpipe[0]);
        close(inpipe[1]);

        execvp(*argv, argv);
        exit(1);
    }

    close(outpipe[1]);

    if (ip) {
        /* escrever da rope no stdin do filho */
        struct outputs o = ovec_get_nth(outputs, ovec_len(outputs) - 1);

        for (size_t i = o.i; i < o.f; i++) {
            struct str * l = rope_get_nth(rope, i);
            write(inpipe[1], str_as_slice(l), str_len(l));
        }

        close(inpipe[1]);
        close(inpipe[0]);
        close(outpipe[1]);
    }

#define _coiso(rope, ch)                 \
    do {                                 \
        struct str * l = str_new();      \
        for (size_t i = 0; i < 3; i++) { \
            str_push(l, '>');            \
        }                                \
        str_push(l, '\n');               \
        rope_push(rope, l);              \
    } while (0)

    _coiso(rope, '>');

    outf = file_fdopen(outpipe[0]);
    ifjmp(outf == NULL, cleanup);

    void * buf = NULL;

    struct outputs o = {0};
    o.i = rope_len(rope);

    for (size_t r = 0; (r = file_readline(outf, &buf)) > 0; buf = NULL) {
        line = str_from_raw_parts(buf, r, r);
        ifjmp(line == NULL, cleanup);

        rope_push(rope, line);
    }

    o.f = rope_len(rope) - 1;
    ovec_push(outputs, o);

    _coiso(rope, '<');
#undef _coiso

cleanup:
    ifnotnull(outf, file_close);
    if (argv != NULL) {
        while (*argv != NULL)
            trfree(*argv);
        trfree(argv);
    }
}

#define is_cmd_line(line) (str_get_nth((line), 0) == '$')

void noob (const char * fname)
{
    struct file * fin = NULL;
    void * buf = NULL;
    struct str * line = NULL;
    struct rope * rope = NULL;
    struct ovec * outputs = NULL;

    fin = file_open(fname, O_RDONLY);
    ifjmp(fin == NULL, ko);

    rope = rope_new();
    ifjmp(rope == NULL, ko);

    outputs = ovec_new();
    ifjmp(outputs == NULL, ko);

    for (size_t r = 0; (r = file_readline(fin, &buf)) > 0; buf = NULL) {
        line = str_from_raw_parts(buf, r, r);
        ifjmp(line == NULL, ko);

        rope_push(rope, line);

        if (is_cmd_line(line))
            process_cmd_line(rope, line, outputs);
    }

ko:
    ifnotnull(fin, file_close);
    ifnotnull(outputs, ovec_free);
    ifnotnull(rope, rope_free);
    ifnotnull(line, str_free);
}

/*
 * Usage:
 *     noob FILE...
 */
int main (int argc, char ** argv)
{
    ifjmp(argc < 2, usage);

    trinit();

    for (int i = 1; i < argc; i++)
        noob(argv[i]);

    treprint();
    trdeinit();

    return 0;

usage:
    return usage(*argv);
}
