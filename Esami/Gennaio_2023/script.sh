#!/bin/bash

i=0
for word in $(cat $1)
do
	if [ $i == 0 ]
	then
		echo "$word" >&1
		i=$((i+1))
	else
		echo "$word" >&2
		i=$((i-1))
	fi
done
