#!/usr/bin/env bash

# ./scripts/uncompress-xz-by-day.sh ./handmaids-tale-105

IDIR=$1
echo $IDIR;

cd $IDIR;

# only extract the JSON data files, not the log files.
for i in ./*.tar.xz;
  do tar xfJ $i;
  echo "$i"; 
done
