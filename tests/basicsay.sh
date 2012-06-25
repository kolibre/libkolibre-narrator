#!/bin/sh

which aplay > /dev/null
if [ $? -ne 0 ] ; then
    echo "Alsa driver is needed to run this test"
    exit 77
fi

./basicsay
