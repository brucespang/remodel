#!/bin/sh
files=$(find src include -name \*.c -o -name \*.h | grep -v scan.c | grep -v parse.c)

# Run in parallel
echo $files | xargs -P16 -n4 $PYTHON bin/cpplint.py \
    --root=include \
    --verbose=2 \
    --filter=-legal/copyright,-readability/casting,-build/include_what_you_use,-whitespace/line_length,-whitespace/braces,-readability/todo,-runtime/int
