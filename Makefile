ifneq (,)
Terribly sorry, this makefile requires gmake.
endif

CC := gcc
LD := gcc

CFLAGS = -Wall -Wextra -Os -g -std=c99 -pedantic
ifeq ($(shell uname -s),OpenBSD)
CFLAGS += -I/usr/local/include
endif

LDFLAGS = -lz
ifeq ($(shell uname -s),OpenBSD)
LDFLAGS += -liconv -L/usr/local/lib
endif

SRCDIR = src
TSTDIR = test
OBJDIR = obj
BINDIR = bin

TESTS := $(wildcard $(TSTDIR)/*.c)
TEST_OBJECTS := $(TESTS:$(TSTDIR)/%.c=$(OBJDIR)/%.o)
SOURCES := $(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c))
OBJECTS := $(filter-out $(OBJDIR)/main.o, $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o))


all: $(BINDIR)/nekopack

$(BINDIR)/nekopack: $(OBJECTS) $(OBJDIR)/main.o
	@mkdir -p $(BINDIR)
	@$(LD) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/main.o -o bin/nekopack

$(BINDIR)/test: $(OBJECTS) $(TEST_OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LD) $(LDFLAGS) $(OBJECTS) $(TEST_OBJECTS) -o $(BINDIR)/test

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.c

$(OBJDIR)/%.o: $(TSTDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c -o $@ -I $(SRCDIR) $(TSTDIR)/$*.c

test: bin/test
	@bin/test

clean:
	rm -f bin/* obj/*

style:
	astyle -n --recursive "src/*.c" "src/*.h" "test/*.c" "test/*.h"

.PHONY: clean test style
