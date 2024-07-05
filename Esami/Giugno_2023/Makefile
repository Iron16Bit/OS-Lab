SHELL := /bin/bash

default:
	@echo "File doesn't exit"
	@exit 1

build: $(FILE)
	@echo "Building..."
	@mkdir $(DIR)
	@gcc $(FILE) -o $(EXE)
	@mv $(EXE) $(DIR)