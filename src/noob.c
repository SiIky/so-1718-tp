#include <ctype.h>

#include <unistd.h>

#include <utils/ifjmp.h>
#include <utils/ifnotnull.h>

#include <file.h>
#include <outputs.h>
#include <rope.h>
#include <str.h>
#include <tralloc.h>

#define stdinfd  0
#define stdoutfd 1

int usage (const char * cmd)
{
    (void) cmd;
    return !0;
}

char ** line_to_argv (struct str * line)
{
    (void) line;
    return NULL;
}

#define is_pipe(line) ((str_get_nth((line), 1) == '|') || isdigit(str_get_nth((line), 1)))

void process_cmd_line (struct rope * rope, struct str * line, struct ovec * outputs)
{
    (void) outputs;
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
    }

    outf = file_fdopen(outpipe[0]);
    ifjmp(outf == NULL, cleanup);

    void * buf = NULL;

    for (size_t r = 0; (r = file_readline(outf, &buf)) > 0; buf = NULL) {
        /* cenas dos outputs (?) */
        line = str_from_raw_parts(buf, r, r);
        ifjmp(line == NULL, cleanup);

        rope_push(rope, line);
    }

cleanup:
    ifnotnull(outf, file_close);
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
