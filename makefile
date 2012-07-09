CC = gcc
CFLAGS = -pedantic -std=c99 -Os
LFLAGS = 

NAME = shellexec
OBJS = shellexec.o
TESTS = test-shellexec.o

all: build test

build: $(OBJS)

test: $(OBJS) $(TESTS)
	@echo Compiled program: test-$(NAME)
	@$(CC) $(LFLAGS) -o test-$(NAME) $^

clean:
	@echo Removing temporary build files
	@rm -f $(OBJS) $(TESTS)

%.o: %.c
	@echo Compiling library: $< "->" $@
	@$(CC) $(CFLAGS) -o $@ -c $<
