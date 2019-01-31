#!/usr/bin/env bash

# PDF multi-render from directory of svg files.
DIR=$1

# inkscape flags
INKFLAGS="--without-gui --export-dpi=90 --export-background-opacity=1.0 --export-background=white"

for i in ${DIR}/*.svg;
do
    ONAME=`echo $i | sed -e 's/svg$/pdf/g'`
    echo $ONAME

    inkscape $i $INKFLAGS --export-pdf=$ONAME;
    if [ $? -eq 0 ]
    then
	echo $i >> audit-pdf-inkscape.out
    else
	cairosvg $i -o $ONAME;
	if [ $? -eq 0 ]
	then
	    echo $i >> audit-pdf-cairosvg.out
	else
	    echo $i >> audit-pdf-fail.out
	fi
    fi

    echo ""
done
