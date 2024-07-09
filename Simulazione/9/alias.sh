#!/bin/bash

# Dato che nessuna variabile deve essere modificata, meglio non usarne. Possiamo prendere il valore in input anche usando $REPLY

# wc -l conta solo le righe

# awk '{print $1}' seleziona solo la prima parola di una stringa e la stampa

alias linecount="read -p 'Inserire path: ' && ([[ -f $REPLY ]] && wc -l $REPLY | awk '{print $1}' || echo '?File not found')";
