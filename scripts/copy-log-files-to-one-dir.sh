#!/usr/bin/env bash

if [ ! -d ./log ]; then
    mkdir log;
fi    
cp browsertime-*.log ./log/;
