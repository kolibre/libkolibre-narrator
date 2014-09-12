#!/bin/sh

${PREFIX} ${bindir:-.}/playfile ${srcdir:-.}/testdata/sample.ogg
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/playfile ${srcdir:-.}/testdata/sample.mp3
result=$?
test $result -eq 0 || exit $result
