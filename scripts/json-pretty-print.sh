#!/usr/bin/bash

# The input file to make pretty, and the generated output filename
IFILE=$1
OFILE=`echo $IFILE | sed 's/.json/.pretty.json/g'`


cat $IFILE | jq '' > $OFILE
