#!/bin/bash

if [[ $# -gt 1 ]];
then
	echo "Usare come ./sub.sh <n>"
	exit 1
fi

if [[ $1 -gt 10 ]];
then
	echo "Passare come argomento un numero fino a 10 (incluso)"
	exit 1
fi

echo "Parent PID = $BASHPID"

count=0
while [[ $count -lt $1 ]];
do
	((count++))
	(echo "Child PID = $BASHPID")
done
