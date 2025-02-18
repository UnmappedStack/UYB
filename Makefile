# Makefile build system for the UYB compiler backend project.
# Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details.

all: build test

build:
	clang src/* -o uyb -I include -g -Wall -Werror

test:
	./uyb test.ssa -o out.S
