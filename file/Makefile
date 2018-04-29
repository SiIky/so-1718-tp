CC=musl-gcc
#CC=gcc

EXEC=example

STD=-std=c11

INCLUDE=\
	-I/usr/local/include/ \

CFLAGS=\
       $(STD)    \
       -Og       \
       -Wall     \
       -Wextra   \
       -g        \
       -pedantic \
       -static   \

SRC=\
    example.c \
    file.c    \

HEADERS=\
	file.h \

OBJS=$(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(INCLUDE) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

cppcheck:
	cppcheck --language=c --enable=all --platform=unix64 --inconclusive -f -v $(INCLUDE) $(SRC) $(HEADERS)

clean:
	$(RM) $(OBJS) $(EXEC)
