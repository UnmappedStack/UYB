VERSION := $(shell git rev-parse --short HEAD)
.PHONY: all build install

all: build install

build:
	@echo "[CMake] Setting up configuration files..."
	@echo "#define COMMIT \"$(VERSION)\"" > include/version.h
	mkdir -p build
	cd build; cmake ..
	@echo "[CMake] Building..."
	cmake --build build

install:
	@echo "[Here] Creating symbolic link in /usr/bin (password may be required)..."
	@if [ ! -e "/usr/bin/uyb" ]; then sudo ln -s $(realpath build/uyb) /usr/bin/uyb; fi
