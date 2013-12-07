#!/bin/sh
if [ $# -gt 0 ] ; then
  files="$*"
else
  files=$(find src include -name \*.c -o -name \*.h)
fi
# Run in parallel
echo $files | xargs -P16 -n4 $PYTHON bin/cpplint.py \
    --root=include \
    --verbose=2 \
    --filter=-legal/copyright,-readability/casting,-build/include_what_you_use,\-whitespace/line_length,\-whitespace/braces
