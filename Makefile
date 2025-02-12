all: build runtest

build:
	clang src/* -o uyb -I include -g -Wall -Werror

runtest:
	./uyb test.ssa -o out.S
