#!/usr/bin/env bash

# NB: Assume MOZPERFAX set in environment
SCRIPT=$MOZPERFAX/scripts/transform-1-metric-to-uno.sh

# Set up lists of metrics to extract.

# Extract list for browsertime .json file.
EDITLIST1=$1
if [ ! -n "$EDITLIST1" ]; then
    EDITLIST1=$MOZPERFAX/data/match-identifier-files/visual-metrics-2021.txt
fi
echo "edit list 1 set to: ${EDITLIST1}";

# Extract list for browsertime .log file.
# Some of the metrics are only in the log files, not in the JSON files.
# (Looking at you, LCP, FCP, etc.)
EDITLIST2=$2
if [ ! -n "$EDITLIST2" ]; then
    EDITLIST2=$MOZPERFAX/data/match-identifier-files/web-vitals-2020-edit.txt
fi
echo "edit list 2 set to: ${EDITLIST2}";



# chrome
if [ -d ./chrome-current ]; then
    $SCRIPT ./chrome-current $EDITLIST1 $EDITLIST2
fi

# firefox
if [ -d ./fenix-current ]; then
    $SCRIPT ./fenix-current $EDITLIST1 $EDITLIST2
fi
