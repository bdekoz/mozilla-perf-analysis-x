#!/usr/bin/bash

#gleaned from cmake VERBOSE=1 all

COMPILEF="-Wall -Wextra -Wfatal-errors -Wno-deprecated-declarations -std=gnu++17 -O2 -g -march=native -pthread -fconcepts"

BASEINCLUDEF="-I/home/bkoz/bin/H-libtorrent/include -I/usr/include/boost"
#A60INCLUDEF="-I/home/bkoz/src/alpha60/src"

LINKF="-lstdc++fs -lboost_system -lboost_date_time -lssl -lcrypto -L/home/bkoz/bin/H-libtorrent/lib/ -ltorrent-rasterbar"
GEOLINKF="-L/usr/lib64/ -lGeoIP"

# The input file to compile, the output filename
CCFILE=$1
EXEFILE=`echo $CCFILE | sed 's/.cc/.exe/g'`

echo "g++ $COMPILEF $INCLUDEF $CCFILE -o ${EXEFILE} $LINKF $GEOLINKF"

g++ $COMPILEF $BASEINCLUDEF $CCFILE -o ${EXEFILE} $LINKF $GEOLINKF
