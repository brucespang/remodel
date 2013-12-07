# Remodel

Make done right.

## Usage

    remodel [file (default build.remodel)]

## Grammar

program ::= production*
production ::= target '<-' dependency (':' '"' command '"")
dependency ::= filename (',' filename)*
target ::= filename (',' filename)*

## Example

DEFAULT <- baz
baz <- foo.o, bar.o: "g++ foo.o bar.o -o baz"
foo.o <- foo.cpp : "g++ -c foo.cpp -o foo.o"
bar.o <- bar.cpp: "g++ -c bar.cpp -o bar.o"
