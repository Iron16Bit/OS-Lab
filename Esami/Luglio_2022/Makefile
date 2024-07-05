SHELL := /bin/bash

# % prende qualsiasi cosa e $@ ritorna il nome del target
% : %.c
	@echo "Building $@..."
	@gcc $@.c -o $@ 

% :
	@echo "$@.c non esiste"