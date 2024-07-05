#!/bin/bash

# Arguments in bash are $1, $2, ... and $# is the number of arguments
[ $# != 10 ] && echo "Must pass exactly 10 arguments" && exit 1

# $@ rende ogni parametro una parola separata, otteniamo quindi una lista di tutte le parole
list=("$@")

# print each word in the list, sort them and put them inside the sorted list
sorted=($(printf '%s\n' "${list[@]}" | sort))
echo ${sorted[@]}