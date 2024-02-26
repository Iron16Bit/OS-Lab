#!/usr/bin/env bash

# Shows current dir content in inverse order

contents=$(pwd|ls)
list=()

for item in $contents; do
    list+=($item)
done

index=${#list[@]}

while [[ $index > 0 ]]; do
    echo "${list[((index-1))]}"
    ((--index))
done