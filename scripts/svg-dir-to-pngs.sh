#!/usr/bin/env bash

# PNG multi-render from directory of svg files.
DIR=$1

# inkscape flags
INKFLAGS="--without-gui --export-dpi=90 --export-background-opacity=1.0 --export-background=white"

for i in ${DIR}/*.svg;
do
    ONAME=`echo $i | sed -e 's/svg$/png/g'`
    echo $ONAME

    inkscape $i $INKFLAGS --export-png=$ONAME;
    if [ $? -eq 0 ]
    then
	echo $i >> audit-png-inkscape.out
    else
	cairosvg $i -o $ONAME;
	if [ $? -eq 0 ]
	then
	    echo $i >> audit-png-cairosvg.out
	else
	    echo $i >> audit-png-fail.out
	fi
    fi

    echo ""
done
