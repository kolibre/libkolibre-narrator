#!/bin/sh

./dbtest
result=$?
rm empty.db
exit $result
