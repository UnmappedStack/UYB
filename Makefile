all: build run

build:
	clang src/* -o thicc -I include -g -Wall -Werror

run:
	./thicc
