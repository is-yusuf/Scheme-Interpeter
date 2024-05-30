#!/bin/bash

# Change "no" to "yes" to use our binaries
USE_BINARIES="no"

# Define sources based on USE_BINARIES flag
if [ "$USE_BINARIES" == "yes" ]; then
    # Function to replace .o with architecture-specific .o files
    replace_arch_specific() {
        echo $1 | sed "s/\.o/-$(arch).o/g"
    }

    SRCS=$(replace_arch_specific "lib/linkedlist.o lib/talloc.o lib/tokenizer.o lib/parser.o main.c interpreter.c")
else
    SRCS="linkedlist.c talloc.c main.c tokenizer.c parser.c interpreter.c"
fi

CC="clang"
CFLAGS="-gdwarf-4 -fPIC"

# Function to determine architecture
arch() {
    uname -m
}

# Default action
default() {
    echo "Available commands: build, compile_target, clean"
}

# Build action
build() {
    $CC $CFLAGS $SRCS -o interpreter
    rm -f *.o
    rm -f vgcore.*
}

# Compile target action
compile_target() {
    target=$1
    if [ -z "$target" ]; then
        echo "No target specified for compile_target"
        return 1
    fi
    $CC $CFLAGS -c $target -o $(echo $target | sed "s/\.c/-$(arch).o/")
}

# Clean action
clean() {
    rm -f *.o
    rm -interpreter
}

# Command line argument processing
case $1 in
    build)
        build
        ;;
    compile_target)
        compile_target $2
        ;;
    clean)
        clean
        ;;
    *)
        default
        ;;
esac