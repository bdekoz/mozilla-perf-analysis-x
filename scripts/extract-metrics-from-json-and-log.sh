#!/usr/bin/env bash

SCRIPTSDIR="${MOZPERFAX}/scripts"
MOZXBDIR="${MOZPERFAX}/bin"

# Top directory of browsertime-results, with nested site sub-directories.
TDIR=$1

# 1 enter working directory
cd $TDIR
$SCRIPTSDIR/copy-json-files-to-one-dir.sh
$SCRIPTSDIR/copy-log-files-to-one-dir.sh

# Edit list for JSON files.
EDITLIST1=$2
#EDITLIST1="${MOZPERFAX}/data/match-identifier-files/visual-metrics-2021.txt"

# Edit list for log files.
EDITLIST2=$3
#EDITLIST2="${MOZPERFAX}/data/match-identifier-files/web-vitals-2020-edit.txt"


# 2, convert json to csv and environment.json files
MOZXBROWSERTIME=moz-perf-x-extract.browsertime.exe
for file in json/*.json
do
    $MOZXBDIR/$MOZXBROWSERTIME $file $EDITLIST1
done
mkdir csv
mv *.csv ./csv;
mv *.environment.json ./csv;

MOZXBROWSERTIMELOG=moz-perf-x-extract.browsertime_log.exe
for file in log/browsertime-*.log
do
    $MOZXBDIR/$MOZXBROWSERTIMELOG $file $EDITLIST2
done
mkdir csv3
mv ./log/*.csv ./csv3;
