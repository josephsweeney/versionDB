CC     = clang
CFLAGS = -g -w -std=c99


build: data

run: build
	./bin/data

data: data.o sha1.o 
	$(CC) bin/$< -o bin/$@

sha1: sha1.o
	$(CC) bin/$< -o bin/$@

%.o: src/%.c
	$(CC) -c $< -o bin/$@ $(CFLAGS)

