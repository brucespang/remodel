# Remodel

[Make done right.](http://plasma.cs.umass.edu/emery/project-2)

## Usage

    ./bin/remodel [file (default build.remodel)]

## Building

Remodel depends on [Concurrency Kit](https://github.com/sbahra/ck), [Flex](http://flex.sourceforge.net/), and [Bison](http://www.gnu.org/software/bison/). After installing the dependencies, run the following:

    make
    ./bin/remodel /path/to/remodel/file

## Testing

There is a set of unit tests in `test/`. They can be run with:

    make test

Code coverage can be checked with

    make clean cov

There are also a few example remodel files that can be run:

    - remodel.remodel - Encodes the dependency graph of the remodel source. It does not build remodel, because there would be too much repetition (since remodel has no variables or pattern matching). It does run two commands: one that echos 1 to remodel1 and 2 to remodel2, and one that just echos 2. Each should only be run once.
    - cycle.remodel - Contains a cycle, useful for checking the cycle detector.

## Grammar

    program ::= production*
    production ::= target '<-' dependency (':' '"' command '"")
    dependency ::= filename (',' filename)*
    target ::= filename (',' filename)*

## Example

    DEFAULT <- baz,bag
    baz,bag <- foo.o, bar.o: "g++ foo.o bar.o -o baz"
    foo.o <- foo.cpp : "g++ -c foo.cpp -o foo.o"
    bar.o <- bar.cpp: "g++ -c bar.cpp -o bar.o"
