CC := gcc

EXEC := noob

STD := -std=c11

INCLUDE := \
    -I.    \
    -Ifile \
    -Irope \

CFLAGS := \
    $(STD)       \
    -Og          \
    -Wall        \
    -Wconversion \
    -Wextra      \
    -g           \
    -pedantic    \
    -static      \

SRC := \
    file/file.c \
    noob.c      \
    outputs.c   \
    rope/rope.c \
    rope/str.c  \

HEADERS := \
    file/file.h \
    outputs.h   \
    rope/rope.h \
    rope/str.h  \

OBJS := $(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(INCLUDE) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

cppcheck:
	cppcheck --language=c --enable=all --platform=unix64 --inconclusive -f -v $(INCLUDE) $(SRC) $(HEADERS)

clean:
	$(RM) $(OBJS) $(EXEC)

.PHONY: all cppcheck clean
