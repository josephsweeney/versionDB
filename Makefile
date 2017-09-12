
build:
	cc src/data.c -o bin/main.o

run: build
	./bin/main.o

