mf-restore: mf-backup
	@rm -r $$FOLDER.bak
	@cp -r $$FOLDER.bak $$FOLDER
	@rm -r $$FOLDER.bak

mf-backup: help
	@cp -r $$FOLDER $$FOLDER.bak

help:
	@echo "This program can make a backup copy of a folder and restore it"