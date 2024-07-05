SHELL := /bin/bash
.DEFAULT_GOAL := build
FILE="NONE"

build:
	@if [[ -f $(FILE) ]]; then\
		echo "file esiste";\
		gcc main.c -o SignalProxy;\
	else\
		echo "File non esiste";\
		touch $(FILE);\
		echo "start\n" > $(FILE);\
		gcc main.c -o SignalProxy;\
	fi\


clean:
	@if [[ -e $(FILE) ]]; then\
		echo "Rimuovo file";\
		rm -r $(FILE);\
		rm -r $(SignalProxy);\
	else\
		echo "Non esiste il backup";\
	fi