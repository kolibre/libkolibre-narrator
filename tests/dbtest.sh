#!/bin/sh

${PREFIX} ${bindir:-.}/dbtest ${srcdir:-.}/testdata/file1.ogg "file one"
result=$?
rm empty.db
exit $result
