@echo off

vmap4extractor.exe -l -d Data

md vmaps

vmap4assembler.exe Buildings vmaps

pause