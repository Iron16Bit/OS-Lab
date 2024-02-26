#!/usr/bin/env bash

# For each item passed as argument, prints it

while [[ $1 != "" ]]; do 
    echo "ARG=$1"
    shift
done