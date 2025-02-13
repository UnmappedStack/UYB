# Makefile build system for the UYB compiler backend project.
# Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details.

all: build runtest

build:
	clang src/* -o uyb -I include -g -Wall -Werror

runtest:
	./uyb test.ssa -o out.S
