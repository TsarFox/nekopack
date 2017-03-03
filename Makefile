ifneq (,)
Terribly sorry, this makefile requires gmake.
endif

CC := gcc
LD := gcc

CFLAGS = -Wall -Wextra -Os
LDFLAGS = -lz

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
	$(LD) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/main.o -o bin/nekopack

$(BINDIR)/test: $(OBJECTS) $(TEST_OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(TEST_OBJECTS) -o $(BINDIR)/test

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.c

$(OBJDIR)/%.o: test/%.c
	$(CC) $(CFLAGS) -c -o $@ -I $(SRCDIR) $(TSTDIR)/$*.c

test: bin/test
	bin/test

clean:
	rm -f bin/* obj/*

.PHONY: clean test
