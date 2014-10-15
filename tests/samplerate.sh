#!/bin/sh

${PREFIX} ${bindir:-.}/samplerate ${srcdir:-.}/testdata/sample_44100.ogg ${srcdir}/testdata/sample_22050.ogg
result=$?
test $result -eq 0 || exit $result
${PREFIX} ${bindir:-.}/samplerate ${srcdir:-.}/testdata/sample_22050.ogg ${srcdir}/testdata/sample_44100.ogg
result=$?
test $result -eq 0 || exit $result
