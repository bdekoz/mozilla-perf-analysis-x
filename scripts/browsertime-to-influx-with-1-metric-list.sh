#!/usr/bin/env bash

# NB: Assume MOZPERFAX set in environment

DEVICEID=$1
PRODUCTID=$2

# Toplevel results directory
RDIR=$3
if [ ! -d $RDIR ]; then
    echo "Cannot find results directory $RDIR"
    exit 1;
else
    echo "Using results directory: $RDIR"
fi

# Set up lists of metrics to extract.
# Metric to extract list for browsertime .log file.
# Some of the metrics are only in the log files, not in the JSON files.
# (Looking at you, LCP, FCP, etc.)
METRICLIST=$4
if [ ! -n "$METRICLIST" ]; then
    METRICLIST="${MOZPERFAX}/data/match-identifier-files/mozilla-alpha-metrics-2021.txt"
fi
echo "Using metric edit list: ${METRICLIST}";

SCRIPT=$MOZPERFAX/scripts/transform-1-metric-cosmology-to-influx.sh

# chrome
if [ "$PRODUCTID" = "chrome" ]; then
    $SCRIPT $DEVICEID $PRODUCTID $RDIR $METRICLIST
fi

# firefox
if [ "$PRODUCTID" = "fenix-nightly" ]; then
    $SCRIPT $DEVICEID $PRODUCTID $RDIR $METRICLIST
fi
