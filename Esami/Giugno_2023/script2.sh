#!/bin/bash

[ $# != 10 ] && echo "Passare esattamente 10 argomenti" && exit 1

[ -f supportoParole.txt ] && rm supportoParole.txt
touch supportoParole.txt

for var in "$@"
do
	echo "$var" >> supportoParole.txt
done

sort supportoParole.txt
