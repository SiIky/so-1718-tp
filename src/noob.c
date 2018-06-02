#include <ctype.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <file.h>
#include <outputs.h>
#include <pids.h>
#include <rope.h>
#include <str.h>
#include <tralloc.h>
#include <utils/common.h>
#include <utils/ifjmp.h>
#include <utils/ifnotnull.h>
#include <utils/unused.h>

#define stdinfd  0
#define stdoutfd 1
#define stderrfd 2

#define merdas() printf("%d\n", __LINE__)

/* master process */
static struct pids * g_pids = NULL;

/* slave processes */
static char *        g_tmpfname = NULL;
static pid_t         g_epid     = -1;
static struct file * g_fin      = NULL;
static struct ovec * g_outputs  = NULL;
static struct rope * g_rope     = NULL;
static void *        g_buf      = NULL;

pid_t killer (pid_t p)
{
    kill(p, SIGINT);
    return -1;
}

void sigint_master_handler (int _sig)
{
    UNUSED(_sig);

    size_t cs = 0;
    struct pids * pids = g_pids;

    if (pids != NULL) {
        cs = pids_len(pids);
        pids_map(pids, killer);
        pids_free(pids);
    }

    for (size_t i = 0; i < cs; i++)
        wait(NULL);

    //treprint();
    trdeinit();

    exit(1);
}

void sigint_slave_handler (int _sig)
{
    UNUSED(_sig);

    pid_t p = g_epid;
    if (p > 0) {
        kill(p, SIGINT);
        waitpid(p, NULL, 0);
    }

    ifnotnull(g_buf,      trfree);
    ifnotnull(g_fin,      file_close);
    ifnotnull(g_outputs,  ovec_free);
    ifnotnull(g_rope,     rope_free);
    ifnotnull(g_tmpfname, unlink);

    //treprint();
    trdeinit();

    exit(1);
}

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

/* TODO: Tratar strings correctamente
 *     "echo \"what? a string?\""
 * deve ser transformado em
 *     ["echo", "\"what? a string?\""]
 * em vez de
 *     ["echo", "\"what?", "a", "string?\""]
 */
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
        trfree(w);
    }

    rope_free(r);
    ret[wc] = NULL;

out:
    return ret;
}

size_t line_to_n (struct str * line, bool iap)
{
    /* deitar fora o '$' */
    char * l = str_as_mut_slice(line) + 1;

    /* procurar pelo '|' */
    char * pipe = strchr(l, '|');
    assert(pipe != NULL);
    *pipe = '\0';

    /* ler o numero */
    int ret = atoi(l);
    if (iap) /* caso seja absoluto */
        ret *= -1;

    *pipe = '|';

    assert(ret >= 0);

    if (ret == 0) /* 0 = 1 = primeiro comando/comando anterior */
        ret = 1;

    return (size_t) ret;
}

#define is_abs_nth_pipe(line) ((str_get_nth((line), 1) == '-') && (isdigit(str_get_nth((line), 2))))
#define is_last_pipe(line)    (str_get_nth((line), 1) == '|')
#define is_rel_nth_pipe(line) (isdigit(str_get_nth((line), 1)))

int process_cmd_line (struct rope * rope, struct str * line, struct ovec * outputs)
{
    /* merdas comuns */
    int ret = 0;
    char ** argv = NULL;
    int inpipe[2];
    int outpipe[2];
    struct file * outf = NULL;
    void * buf = NULL;

    bool iap = is_abs_nth_pipe(line);
    bool ilp = is_last_pipe(line);
    bool irp = is_rel_nth_pipe(line);

    bool inp = iap || irp;
    bool ip = ilp || inp;

    pipe(outpipe);

    ifjmp((ovec_is_empty(outputs) && ip), ko);

    if (ip) /* merdas pro pipe */
        pipe(inpipe);

    argv = line_to_argv(line);
    ifjmp(argv == NULL, ko);

    /* executar */
    pid_t c = fork();
    ifjmp(c == -1, ko);

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

    g_epid = c;

    close(outpipe[1]);

    if (ip) {
        /* escrever da rope no stdin do filho */
        size_t len = ovec_len(outputs);
        size_t idx = len - 1; /* para o caso "$|" */

        if (inp) { /* caso nao seja "$|" */
            size_t n = line_to_n(line, iap);
            n = min(n, ovec_len(outputs));

            idx = (irp) ?
                len - n: /* caso seja relativo */
                n;       /* caso seja absoluto */
        }

        struct outputs o = ovec_get_nth(outputs, idx);

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
    ifjmp(outf == NULL, ko);

    struct outputs o;
    o.i = rope_len(rope);

    for (size_t r = 0; (r = file_readline(outf, &buf)) > 0; buf = NULL) {
        g_buf = buf;
        line = str_from_raw_parts(buf, r, r);
        ifjmp(line == NULL, ko);
        g_buf = NULL;

        rope_push(rope, line);
    }

    o.f = rope_len(rope);
    ovec_push(outputs, o);

    _coiso(rope, '<');
#undef _coiso
    int st;
    wait(&st);
    g_epid = -1;

    ifjmp(!WIFEXITED(st), ko);
    ifjmp(WEXITSTATUS(st) != 0, ko);
out:
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

    return ret;

ko:
    ret = 1;
    goto out;
}
#undef is_abs_nth_pipe
#undef is_last_pipe
#undef is_rel_nth_pipe

enum LTYPE {
    LTYPE_CMD,
    LTYPE_OP_ST,
    LTYPE_OP_MID,
    LTYPE_OP_END,
    LTYPE_TEXT,
    LTYPE_INVALID,
};

enum LTYPE ltype (const struct str * line)
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
        ret = (mid) ?
            LTYPE_OP_MID :
            LTYPE_TEXT;
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
    int tmpfd = -1;

    fin = file_open(fname, O_RDONLY);
    ifjmp(fin == NULL, ko);
    g_fin = fin;

    rope = rope_new();
    ifjmp(rope == NULL, ko);
    g_rope = rope;

    outputs = ovec_new();
    ifjmp(outputs == NULL, ko);
    g_outputs = outputs;

    for (size_t r = 0; (r = file_readline(fin, &buf)) > 0; buf = NULL) {
        struct str * line = str_from_raw_parts(buf, r, r);
        ifjmp(line == NULL, ko);

        switch (ltype(line)) {
            case LTYPE_CMD:
                rope_push(rope, line);
                int r = process_cmd_line(rope, line, outputs);
                ifjmp(r != 0, ko);
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

    char tmpfname[] = "/tmp/noob.XXXXXX";
    tmpfd = mkstemp(tmpfname);
    ifjmp(tmpfd == -1, ko);

    g_tmpfname = tmpfname;

    for (rope_iter(rope); rope_itering(rope); rope_iter_next(rope)) {
        struct str * l = rope_get_nth(rope, rope_iter_idx(rope));

        write(tmpfd, str_as_slice(l), str_len(l));
    }

    int rn = rename(tmpfname, fname);
    ifjmp(rn == -1, ko);

out:
    g_fin = NULL;
    g_outputs = NULL;
    g_rope = NULL;
    g_tmpfname = NULL;
    g_buf = NULL;

    ifnotnull(buf, trfree);
    ifnotnull(fin, file_close);
    ifnotnull(outputs, ovec_free);
    ifnotnull(rope, rope_free);

    if (tmpfd != -1) close(tmpfd);

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

    int ret = 0;

    signal(SIGINT, sigint_master_handler);
    trinit();

    struct pids * pids = pids_with_capacity(((size_t) argc) - 1);
    ifjmp(pids == NULL, ko);
    g_pids = pids;

    int cs = 0;
    for (int i = 1; i < argc; i++) {
        pid_t c = fork();

        if (c == -1)
            continue;

        if (c == 0) {
#if 0
            {
                size_t len = strlen(argv[i]);
                argv[i][len] = '\n';

                write(stderrfd, argv[i], len + 1);

                argv[i][len] = '\0';
            }
#endif

            pids_free(pids);
            g_pids = NULL;

            signal(SIGINT, sigint_slave_handler);
            ret = noob(argv[i]);
            goto out;
        }

        cs++;
    }

    for (int i = 0; i < cs; i++) {
        int st = 0;

        pid_t c = waitpid(-1, &st, 0);

        if (c == -1) {
            ret++;
            continue;
        }

        {
            size_t idx = pids_find(pids, c);

            if (idx < pids_len(pids))
                pids_remove(pids, idx);
        }

        if (WIFEXITED(st))
            ret += WEXITSTATUS(st);
        else
            ret++;
    }

out:
    pids_free(pids);
    g_pids = NULL;
    treprint();
    trdeinit();

    return ret;

ko:
    ret++;
    goto out;

usage:
    return usage();
}
