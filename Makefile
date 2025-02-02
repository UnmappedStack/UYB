all: build run

build:
	clang src/* -o uyb -I include -g -Wall -Werror

run:
	./uyb
