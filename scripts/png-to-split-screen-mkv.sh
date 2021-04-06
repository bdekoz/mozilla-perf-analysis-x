#!/usr/bin/env bash

MOZDIR=$1
CHROMEDIR=$2
SEC=$3

if [ ! -n "$MOZDIR" ]; then
    echo "Mozilla results directory argument not supplied, exiting";
    exit 1;
fi
if [ ! -n "$CHROMEDIR" ]; then
    echo "Chrome results directory argument not supplied, exiting";
    exit 2;
fi
if [ ! -n "$SEC" ]; then
    echo "Seconds for each clip argument not supplied, exiting";
    exit 3;
fi

# Assume json to svg, png/pdf transforms prior

# 1: Convert png to movie
MILDIR=/home/bkoz/src/MiL.git/scripts
PNG2MKV=ffmpeg-from-png-dir-to-mkv-sec.sh

$MILDIR/$PNG2MKV $MOZDIR/png $SEC
$MILDIR/$PNG2MKV $CHROMEDIR/png $SEC

# 2: Take outputs and combine.
MOZMKV=$MOZDIR/png-${SEC}.mkv
CHROMEMKV=$CHROMEDIR/png-${SEC}.mkv
MKVCOMPOSITE=ffmpeg-compress-2-channel-mosaic.sh
$MILDIR/$MKVCOMPOSITE $CHROMEMKV $MOZMKV 


# 3: Rename to something else.
mv merge-2-channel.mkv fenix-v-chrome.`date --iso`.1080p.mkv
