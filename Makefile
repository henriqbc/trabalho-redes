PROJECT_NAME := trabalho-redes-2-chat

CC = gcc

INCLUDE_FOLDER = include
SOURCE_FOLDER = src

BINARY = ./main
PROG = main.c

SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
OBJECTS = $(SOURCES:.c=.o)

LIBRARIES_TO_LINK = -lm

FLAGS = -g -Wall -Werror
VFLAGS = --show-leak-kinds=all --track-origins=yes --leak-check=full -s

# ------------------------------------------------------------------------------

all: $(BINARY)

run: $(BINARY)
	$(BINARY)

# COMPILING

compile: $(SOURCES) $(PROG)
	for source in $(SOURCES); do \
		$(CC) -c $$source -o $${source%.c}.o -I $(INCLUDE_FOLDER) $(FLAGS); \
	done

$(PROG:.c=.o): $(PROG)
	$(CC) -c $^ -o $@ -I $(INCLUDE_FOLDER) $(FLAGS)

src/%.o: src/%.c
	$(CC) -c $? -o $@ -I $(INCLUDE_FOLDER) $(FLAGS)


# LINKING

link $(BINARY): $(OBJECTS) $(PROG:.c=.o)
	$(CC) $^ -o $(BINARY) $(LIBRARIES_TO_LINK) $(FLAGS)


# UTILS

val: all
	valgrind $(VFLAGS) $(BINARY)

zip:
	zip -r $(PROJECT_NAME).zip $(PROG) $(INCLUDE_FOLDER) $(SOURCE_FOLDER) Makefile

clean:
	rm $(BINARY) $(OBJECTS)
