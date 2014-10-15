#!/bin/sh

${PREFIX} ${bindir:-.}/monostereo ${srcdir:-.}/testdata/sample_mono.ogg
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/monostereo ${srcdir:-.}/testdata/sample_stereo.ogg
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/monostereo ${srcdir:-.}/testdata/sample_mono.mp3
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/monostereo ${srcdir:-.}/testdata/sample_stereo.mp3
result=$?
test $result -eq 0 || exit $result
