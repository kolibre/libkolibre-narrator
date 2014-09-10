#!/bin/sh

${PREFIX} ${bindir:-.}/playfile ${srcdir:-.}/testdata/file1.ogg
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/playfile ${srcdir:-.}/testdata/file2.ogg
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/playfile ${srcdir:-.}/testdata/file3.ogg
result=$?
test $result -eq 0 || exit $result
