CC     = clang
CFLAGS = -g -std=c99
OBJS = bin/server.o bin/data.o bin/sha1.o bin/commit.o
PROG = main

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bin/$@

run: all
	./bin/main

server: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bin/$@

client: bin/client.o
	$(CC) $(CFLAGS) $< -o bin/$@

bin/data.o: src/sha1.h src/commit.c src/data.c
	$(CC) $(CFLAGS) -c src/data.c -o $@

bin/sha1.o: src/sha1.h
	$(CC) $(CFLAGS) -c src/sha1.c -o $@

bin/commit.o: src/commit.h src/commit.c
	$(CC) $(CFLAGS) -c src/commit.c -o $@

bin/server.o: src/server.h src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o $@

bin/client.o: src/client.h src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o $@



clean:
	rm bin/*
