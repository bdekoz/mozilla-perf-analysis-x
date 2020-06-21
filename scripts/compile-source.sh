#!/usr/bin/bash

#gleaned from cmake VERBOSE=1 all

COMPILEF="-Wall -Wextra -Wfatal-errors -Wno-deprecated-declarations -std=gnu++17 -O2 -g -march=native -pthread -fconcepts"

BASEINCLUDEF="-I/usr/include/boost -I/home/bkoz/src/izzi"
INCLUDEF=$BASEINCLUDEF

BASELINKF="-lstdc++fs -lssl -lcrypto"
BOOSTLINKF="-lboost_system -lboost_date_time"
TORRENTLINKF="-L/home/bkoz/bin/H-libtorrent/lib/ -ltorrent-rasterbar"
GEOLINKF="-L/usr/lib64/ -lGeoIP"
LINKF=$BASELINKF

# The input file to compile, the output filename
CCFILE=$1
EXEFILE=`echo $CCFILE | sed 's/.cc/.exe/g'`

echo "g++ $COMPILEF $INCLUDEF $CCFILE -o ${EXEFILE} $LINKF"

g++ $COMPILEF $INCLUDEF $CCFILE -o ${EXEFILE} $LINKF
