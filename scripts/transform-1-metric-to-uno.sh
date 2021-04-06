#!/usr/bin/env bash

SCRIPTSDIR="${MOZPERFAX}/scripts"
MOZXBDIR="${MOZPERFAX}/bin"

# Top directory of browsertime-results, with nested site sub-directories.
TDIR=$1
EDITLIST1=$2
EDITLIST2=$3

$SCRIPTSDIR/extract-metrics-from-json-and-log.sh $TDIR $EDITLIST1 $EDITLIST2

mypwd=`pwd`
echo "pwd is: $mypwd"

# 3 convert to svg
MOZV=moz-perf-x-analyze-radial-uno.exe
for file in ${TDIR}/csv/*.csv
do
    echo $file
    $MOZXBDIR/$MOZV $file "Visual Metrics 2021"
done

if [ ! -d ./svg ]; then
    mkdir svg
fi
mv *.svg ./svg;


# 4 convert to png
TOPNG=$SCRIPTSDIR/svg-dir-to-pngs.sh
$TOPNG ./svg;

if [ ! -d ./png ]; then
    mkdir png;
fi
mv ./svg/*.png ./png;
