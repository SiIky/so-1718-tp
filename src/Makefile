CC := gcc

EXEC := notebook

STD := -std=c11

# enable alloc tracing
TRALLOC_TRACE := -DTR_TRACE

# use tralloc for all vec implementations
VEC_ALLOCS := \
    -D_VEC_CALLOC=trcalloc   \
    -D_VEC_FREE=trfree       \
    -D_VEC_MALLOC=trmalloc   \
    -D_VEC_REALLOC=trrealloc \

INCLUDE := \
    -I.       \
    -Ifile    \
    -Irope    \
    -Itralloc \

CFLAGS := \
    $(STD)              \
    $(TRALLOC_TRACE)    \
    $(VEC_ALLOCS)       \
    -D_XOPEN_SOURCE=500 \
    -Og                 \
    -Wall               \
    -Wconversion        \
    -Wextra             \
    -g                  \
    -pedantic           \

SRC := \
    file/file.c        \
    noob.c             \
    outputs.c          \
    pids.c             \
    rope/rope.c        \
    rope/str.c         \
    tralloc/tralloc.c  \
    tralloc/trs.c      \
    tralloc/trs_vec.c  \
    tralloc/trstrdup.c \

HEADERS := \
    file/file.h        \
    outputs.h          \
    pids.h             \
    rope/rope.h        \
    rope/str.h         \
    tralloc/tralloc.h  \
    tralloc/trs.h      \
    tralloc/trs_vec.h  \
    tralloc/trstrdup.h \

OBJS := $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(INCLUDE) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

ZIP := entrega.zip
RELATORIO := relatorio.pdf

entrega: $(ZIP)

$(ZIP): $(SRC) $(HEADERS) Makefile $(RELATORIO)
	$(RM) $(ZIP)
	zip -r -n: -9 $(ZIP) $(SRC) $(HEADERS) Makefile $(RELATORIO) *.nob

clean:
	$(RM) $(OBJS) $(EXEC) $(ZIP)

.PHONY: all cppcheck clean entrega
