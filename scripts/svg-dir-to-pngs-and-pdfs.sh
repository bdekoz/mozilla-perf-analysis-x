#!/usr/bin/env bash

# PNG and PDF multi-render from directory of svg files.
DIR=$1

SCRIPTSDIR=/home/bkoz/src/alpha60/scripts

$SCRIPTSDIR/svg-dir-to-pngs.sh ${DIR};
touch ./done-png;

$SCRIPTSDIR/svg-dir-to-pdfs.sh ${DIR};
touch ./done-pdf;
