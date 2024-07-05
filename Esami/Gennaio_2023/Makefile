SHELL := /bin/bash

FILE := log.txt

build: SignalProxy.c
	@echo "Buildando il progetto..."
	@gcc SignalProxy.c -o SignalProxy
	@[ ! -e $(FILE) ] && echo "Creazione di $(FILE)..." && touch $(FILE) && echo "start\n" > $(FILE) || echo "$(FILE) esiste già..."

clean:
	@echo "Rimuovendo tutti i file..."
	@[ -e $(FILE) ] && rm $(FILE)
	@[ -e SignalProxy ] && rm SignalProxy