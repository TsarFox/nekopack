SHELL = /bin/sh

.SUFFIXES =
.SUFFIXES = .c .o

CC := gcc
LD := gcc
CFLAGS := -Wall -Wextra -Os -funroll-all-loops -std=gnu99 -pedantic
LDFLAGS := -lz
STRIPARGS := -s -p

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


all: $(BINDIR)/nekopack


$(BINDIR)/nekopack: $(OBJECTS)
	@mkdir -p $(BINDIR)
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -o $@ -c $<


.PHONY: strip
strip:
	@strip $(STRIPARGS) $(BINDIR)/nekopack


.PHONY: clean
clean:
	@rm -rf $(OBJECTS)


.PHONY: remove
remove: clean
	@rm -rf $(BINDIR)/$(TARGET)
