#!/bin/sh
ctags -R
find . -name "*.h" -o -name "*.c" -o -name "*.cpp" > cscope.files
cscope -bkq
