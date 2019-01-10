#!/usr/bin/bash

FILE=$1
OFILE=`echo $CCFILE | sed 's/.jsonlz4/.json/g'`

./mozlz4a.py -d $FILE $OFILE
