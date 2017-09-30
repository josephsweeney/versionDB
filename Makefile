CC     = clang
CFLAGS = -g -std=c99
OBJS = bin/data.o bin/sha1.o
PROG = main

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bin/$@

run: all
	./bin/main

bin/data.o: src/sha1.h src/data.c
	$(CC) $(CFLAGS) -c src/data.c -o $@

bin/sha1.o: src/sha1.h
	$(CC) $(CFLAGS) -c src/sha1.c -o $@


clean:
	rm bin/*
