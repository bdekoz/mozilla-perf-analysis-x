#!/usr/bin/env bash

# NB: Assume MOZPERFAX set in environment
SCRIPT=$MOZPERFAX/scripts/transform-1-metric-cosmology-to-uno.sh

# Set up lists of metrics to extract.
# Metric to extract list for browsertime .log file.
# Some of the metrics are only in the log files, not in the JSON files.
# (Looking at you, LCP, FCP, etc.)
METRICLIST=$1
if [ ! -n "$METRICLIST" ]; then
    METRICLIST=$MOZPERFAX/data/match-identifier-files/web-vitals-2020-edit.txt
fi
echo "metric edit list set to: ${METRICLIST}";


# chrome
if [ -d ./chrome-current ]; then
    $SCRIPT ./chrome-current $METRICLIST
fi

# firefox
if [ -d ./fenix-current ]; then
    $SCRIPT ./fenix-current $METRICLIST
fi
