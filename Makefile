CFLAGS = -Wall -Wextra -Os
LDFLAGS = -lz

OBJ = obj/cli.o obj/compress.o obj/crypto.o obj/encoding.o obj/header.o obj/io.o obj/table.o
TEST_OBJ = obj/test_cli.o obj/test_encoding.o obj/test_header.o obj/test_io.o obj/test_table.o obj/test_run.o
ENTRY_OBJ = obj/main.o


all: bin/nekopack

bin/nekopack: $(OBJ) $(ENTRY_OBJ)
	cc $(LDFLAGS) $(OBJ) $(ENTRY_OBJ) -o bin/nekopack

test: bin/test
	bin/test

bin/test: $(OBJ) $(TEST_OBJ)
	cc $(LDFLAGS) $(OBJ) $(TEST_OBJ) -o bin/test

obj/%.o: src/%.c
	cc $(CFLAGS) -c -o $@ src/$*.c

obj/%.o: test/%.c
	cc -I src $(CFLAGS) -c -o $@ test/$*.c
