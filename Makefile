ifneq (,)
Terribly sorry, this makefile requires gmake.
endif

CC := gcc
LD := gcc

CFLAGS = -Wall -Wextra -Os -std=c99 -pedantic -fstack-protector-all
CFLAGS += $(USER_CFLAGS)
ifeq ($(shell uname -s),OpenBSD)
CFLAGS += -I/usr/local/include
endif

LDFLAGS = -lz
LDFLAGS += $(USER_LDFLAGS)
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
	@echo "  LD    $(@:$(BINDIR)/%=%)"
	@$(LD) $(CFLAGS) $(OBJECTS) $(OBJDIR)/main.o -o $(BINDIR)/nekopack $(LDFLAGS)

$(BINDIR)/test: $(OBJECTS) $(TEST_OBJECTS)
	@mkdir -p $(BINDIR)
	@echo "  LD    $(@:$(BINDIR)/%=%)"
	@$(LD) $(CFLAGS) $(OBJECTS) $(TEST_OBJECTS) -o $(BINDIR)/test $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "  CC    $(@:$(OBJDIR)/%=%)"
	@$(CC) $(CFLAGS) -c -o $@ $(SRCDIR)/$*.c

$(OBJDIR)/%.o: $(TSTDIR)/%.c
	@mkdir -p $(OBJDIR)
	@echo "  CC    $(@:$(OBJDIR)/%=%)"
	@$(CC) $(CFLAGS) -c -o $@ -I $(SRCDIR) $(TSTDIR)/$*.c

test: bin/test
	@bin/test

clean:
	rm -f bin/* obj/*

.PHONY: clean test
