@ECHO OFF
title ExtractorForUniverse
COLOR 05

CLS
ECHO.
ECHO         _____             __                         
ECHO        / ___/__  ______  / /_  ________  ____  ____ _
ECHO        \__ \/ / / / __ \/ __ \/ ___/ _ \/ __ \/ __ `/
ECHO       ___/ / /_/ / /_/ / / / / /  /  __/ / / / /_/ / 
ECHO      /____/\__, / .___/_/ /_/_/   \___/_/ /_/\__,_/  
ECHO           /____/_/                                   
ECHO.
ECHO.
ECHO   		Project UniverseEmu 2026(c)
ECHO.
ECHO.
ECHO          Compilateur DBCs/MAPS/VMAPS/MMAPS pour Universe
ECHO.
ECHO.
PAUSE

:Extract
CLS
COLOR 05
ECHO.
ECHO L'extraction des DBCs et des MAPS est en cours...
ECHO.
mapextractor.exe
ECHO.
ECHO L'extraction des DBCs et des MAPS est terminer... Extraction des VMAPS en cours...
ECHO.
vmap4extractor.exe
ECHO.
ECHO L'extraction des VMAPS est terminer... Assemblage des VMAPS en cours...
ECHO.
MKDIR vmaps
vmap4assembler.exe Buildings vmaps
ECHO.
ECHO.
ECHO L'assemblage des VMAPS est terminer... Construction des MMAPS en cours...
ECHO.
ECHO.
MKDIR mmaps
MOVE .\buildings\* .\vmaps
mmaps_generator.exe
PAUSE

CLS
ECHO.
ECHO La compilation des DBCs/MAPS/VMAPS/MMAPS est terminer !
ECHO.
ECHO ExtractorForUniverse V4.00 2022(c)
ECHO.
ECHO.
ECHO.
PAUSE
:EXIT