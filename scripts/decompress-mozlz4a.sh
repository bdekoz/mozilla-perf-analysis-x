#!/usr/bin/bash

SCRIPTSDIR=$MOZTELEMETRYX/scripts
FILE=$1
OFILE=`echo $FILE | sed 's/.jsonlz4/.json/g'`

$SCRIPTSDIR/mozlz4a.py -d $FILE $OFILE
