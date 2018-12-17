#!/usr/bin/bash

# The input YAML file to convert to JSON
IFILE=$1
OFILE=`echo $IFILE | sed 's/.yaml/.json/g'`

yq . $IFILE >& $OFILE

echo "converted to $IFILE to $OFILE"
