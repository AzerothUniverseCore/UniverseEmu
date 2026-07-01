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

:MENU
ECHO.
ECHO 1 - Extraction des DBCs et des MAPS
ECHO 2 - Extraction des VMAPS/Assemblage des VMAPS
ECHO 3 - Construction des MMAPS
ECHO 4 - Extraction/Assemblage/Construction des DBCs/MAPS/VMAPS/MMAPS
ECHO 5 - Quitter
ECHO.
SET /P M=1 DBCs MAPS, 2 VMAPS, 3 MMAPS, 4 DBCs/MAPS/VMAPS/MMAPS :
IF %M%==1 GOTO MAPS
IF %M%==2 GOTO VMAPS
IF %M%==3 GOTO MMAPS
IF %M%==4 GOTO ALL
IF %M%==5 GOTO :EOF

:MAPS
start /b /w mapextractor.exe
GOTO MENU

:VMAPS
start /b /w vmap4extractor.exe
start /b /w vmap4assembler.exe Buildings vmaps
rmdir Buildings /s /q
GOTO MENU

:MMAPS
start /b /w mmaps_generator.exe
GOTO MENU

:ALL
start /b /w mapextractor.exe
start /b /w vmap4extractor.exe
start /b /w vmap4assembler.exe
rmdir Buildings /s /q
start /b /w mmaps_generator.exe
GOTO MENU