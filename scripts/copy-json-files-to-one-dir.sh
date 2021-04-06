#!/usr/bin/env bash

if [ ! -d ./json ]; then
    mkdir json;
fi
cp `find . -type f -name "*.json"` ./json/;
