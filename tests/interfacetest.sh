#!/bin/sh

toppkgdir=${srcdir:-.}
utils=$toppkgdir/../utils/build_message_db.py
prompts=$toppkgdir/../prompts/narrator.csv
messages=$toppkgdir/../prompts/types.csv
translations=$toppkgdir/../prompts/sv_translations.csv
language=sv
database=narrator.db

which aplay > /dev/null
if [ $? -ne 0 ] ; then
    echo "Alsa driver is needed to run this test"
    exit 77
fi

python $utils -p $prompts -m $messages -t $translations -l $language -o $database

./interfacetest $language $database
rm $database
