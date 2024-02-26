#!/usr/bin/env bash

# Returns all arguments in inverse order

list=()

while [[ $1 != "" ]]; do 
    list+=($1)
    shift
done

index=${#list[@]}

while [[ $index > 0 ]]; do
    echo "${list[((index-1))]}"
    ((--index))
done