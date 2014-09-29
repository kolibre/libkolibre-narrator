#!/bin/sh

${PREFIX} ${bindir:-.}/dbtest ${srcdir:-.}/testdata/file1.ogg "file one"
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/dbtest ${srcdir:-.}/testdata/file2.mp3 "file two"
result=$?
test $result -eq 0 || exit $result
