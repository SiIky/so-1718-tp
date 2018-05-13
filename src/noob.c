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
#define stderrfd 2

#define merdas() printf("%d\n", __LINE__)

int usage (void)
{
    char usg[] = "Usage: noob FILE...\n";
    write(stderrfd, usg, sizeof(usg) / sizeof(*usg));
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
    ////        str_push(w, '\0');

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
        trfree(w);
    }

    rope_free(r);
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
    void * buf = NULL;

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

        if (ip) {
            dup2(inpipe[0], stdinfd);
            close(inpipe[0]);
            close(inpipe[1]);
        }

        close(outpipe[0]);
        close(outpipe[1]);

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
    }

#define _coiso(rope, ch)                 \
    do {                                 \
        struct str * l = str_new();      \
        for (size_t i = 0; i < 3; i++) { \
            str_push(l, ch);             \
        }                                \
        str_push(l, '\n');               \
        rope_push(rope, l);              \
    } while (0)

    _coiso(rope, '>');

    outf = file_fdopen(outpipe[0]);
    ifjmp(outf == NULL, cleanup);

    struct outputs o;
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
    ifnotnull(buf, trfree);
    if (argv != NULL) {
        size_t i = 0;
        while (argv[i] != NULL) {
            trfree(argv[i]);
            i++;
        }
        trfree(argv);
    }
}

enum LTYPE {
    LTYPE_CMD,
    LTYPE_OP_ST,
    LTYPE_OP_MID,
    LTYPE_OP_END,
    LTYPE_TEXT,
    LTYPE_INVALID,
};

enum LTYPE ltype (struct str * line)
{
    static bool mid = false;
    char begin[] = ">>>";
    char end[] = "<<<";
    enum LTYPE ret = LTYPE_INVALID;

    if (str_get_nth((line), 0) == '$') {
        ret = LTYPE_CMD;
    } else if (str_len(line) == 4) {
        if (strncmp(begin, str_as_slice(line), 3) == 0) {
            ret = LTYPE_OP_ST;
            mid = true;
        } else if (strncmp(end, str_as_slice(line), 3) == 0) {
            ret = LTYPE_OP_END;
            mid = false;
        } else {
            ret = (mid) ?
                LTYPE_OP_MID :
                LTYPE_TEXT ;
        }
    } else {
        ret = LTYPE_TEXT;
    }

    return ret;
}

int noob (const char * fname)
{
    int ret = 0;
    struct file * fin = NULL;
    void * buf = NULL;
    struct rope * rope = NULL;
    struct ovec * outputs = NULL;

    fin = file_open(fname, O_RDONLY);
    ifjmp(fin == NULL, ko);

    rope = rope_new();
    ifjmp(rope == NULL, ko);

    outputs = ovec_new();
    ifjmp(outputs == NULL, ko);

    for (size_t r = 0; (r = file_readline(fin, &buf)) > 0; buf = NULL) {
        struct str * line = str_from_raw_parts(buf, r, r);
        ifjmp(line == NULL, ko);

        switch (ltype(line)) {
            case LTYPE_CMD:
                rope_push(rope, line);
                process_cmd_line(rope, line, outputs);
                break;
            case LTYPE_TEXT:
                rope_push(rope, line);
                break;
            case LTYPE_OP_ST:
            case LTYPE_OP_MID:
            case LTYPE_OP_END:
                str_free(line);
                break;
            case LTYPE_INVALID:
            default:
                goto ko;
                break;
        }
    }


//    for (rope_iter(rope); rope_itering(rope); rope_iter_next(rope)) {
//        struct str * l = rope_get_nth(rope, rope_iter_idx(rope));
//        str_set_nth(l, str_len(l) - 1, '\0');
//        printf("%s\n", str_as_slice(l));
//    }
char filename[] = "/tmp/mytemp.noob"; 
int fd = mkstemp(filename);
    for (rope_iter(rope); rope_itering(rope); rope_iter_next(rope)) {
        struct str * l = rope_get_nth(rope, rope_iter_idx(rope));
//        str_set_nth(l, str_len(l) - 1, '\0');
        
        write(fd,str_as_slice(l),str_len(l));
//        write(fd,"\n",2);
    }

//    int pid = fork();
//    if (!pid) {
//        execlp("mv","mv",filename,fname,(char*)0);
//    }
int rn = rename(filename, fname);
out:
    ifnotnull(buf, trfree);
    ifnotnull(fin, file_close);
    ifnotnull(outputs, ovec_free);
    ifnotnull(rope, rope_free);
    return ret;

ko:
    ret = 1;
    goto out;
}

/*
 * Usage:
 *     noob FILE...
 */
int main (int argc, char ** argv)
{
    ifjmp(argc < 2, usage);

    trinit();

    int ret = 0;
    for (int i = 1; i < argc; i++)
        ret += noob(argv[i]);

    treprint();
    trdeinit();

    return ret;

usage:
    return usage();
}
