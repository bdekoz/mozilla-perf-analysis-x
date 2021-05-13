#!/usr/bin/env bash

if [ -d ./json ]; then
    rm -rf ./json;
fi

JFILES=`find . -type f -name "*.json"`
if [ -z "$JFILES" ]; then
    echo "no json files found in directory, exiting"
    exit 11;
else
   mkdir ./json;
   cp ${JFILES} ./json/;
fi
