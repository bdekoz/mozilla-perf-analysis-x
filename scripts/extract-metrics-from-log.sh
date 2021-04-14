#!/usr/bin/env bash

SCRIPTSDIR="${MOZPERFAX}/scripts"
MOZXBDIR="${MOZPERFAX}/bin"

# Top directory of browsertime-results, with nested site sub-directories.
RDIR=$1

# 1 enter working directory
cd $RDIR
$SCRIPTSDIR/copy-log-files-to-one-dir.sh

# Edit list for log files.
EDITLIST2=$2
#EDITLIST2="${MOZPERFAX}/data/match-identifier-files/web-vitals-2020-edit.txt"


# 2, convert json to csv and environment.json files
MOZXBROWSERTIMELOG=moz-perf-x-extract.browsertime_log.exe
for file in log/browsertime-*.log
do
    $MOZXBDIR/$MOZXBROWSERTIMELOG $file $EDITLIST2
done
mkdir csv3
mv ./log/*.csv ./csv3;
