CC = gcc
CFLAGS = -g -Wall -Wvla -Werror -fsanitize=address,undefined
CLEAN = rm -f
WORDS = ./words

# Executables
words: words.o
    $(CC) $(CFLAGS) -o words words.o

# Object files
words.o: words.c
    $(CC) $(CFLAGS) -c words.c

# Targets
all: words

clean:
    $(CLEAN) words words.o

run_example1: words
    $(WORDS) example1.txt

run_example2: words
    $(WORDS) example2.txt