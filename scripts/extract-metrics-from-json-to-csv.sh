#!/usr/bin/env bash

SCRIPTSDIR="${MOZPERFAX}/scripts"
MOZXBDIR="${MOZPERFAX}/bin"

# Top directory of browsertime-results, with nested site sub-directories.
RDIR=$1

# 1 enter working directory
cd $RDIR
$SCRIPTSDIR/copy-json-files-to-one-dir.sh

# Edit list for JSON files.
EDITLIST1=$2
#EDITLIST1="${MOZPERFAX}/data/match-identifier-files/visual-metrics-2021.txt"

# 2, convert json to csv and environment.json files
MOZXBROWSERTIME=moz-perf-x-extract.browsertime.exe
for file in json/browsertime*.json
do
    $MOZXBDIR/$MOZXBROWSERTIME $file $EDITLIST1
done
mkdir csv
mv *.csv ./csv;

mv *.environment.json ./json;
