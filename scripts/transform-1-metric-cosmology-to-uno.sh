#!/usr/bin/env bash

# Top directory of browsertime-results, with nested site sub-directories.
RDIR=$1

METRICLIST=$2
mfilestem=${METRICLIST%.*}
mcosmo=${mfilestem##*/}

mypwd=`pwd`

echo "current directory is: $mypwd"
echo "tdir is: $RDIR"
echo "metric cosmology is: $mcosmo"

SCRIPTSDIR="${MOZPERFAX}/scripts"
MOZXBDIR="${MOZPERFAX}/bin"

# 1a extract log metrics (based on metriclist) to csv
$SCRIPTSDIR/extract-metrics-from-log.sh $RDIR $METRICLIST

# 1b extract json metrics (all) to csv and environment.json
$SCRIPTSDIR/extract-metrics-from-json.sh $RDIR

# CSVDIR is the active results directory.
# CSVDIR is a directory of 2 or 3 item csv's from browsertime json files.
CSVDIR=csv3
#CSVDIR=csv

# 2 convert csv to svg
MOZV=moz-perf-x-analyze-radial-uno.exe
for file in ${RDIR}/${CSVDIR}/*.csv
do
    echo $file
    $MOZXBDIR/$MOZV $file $mcosmo
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
