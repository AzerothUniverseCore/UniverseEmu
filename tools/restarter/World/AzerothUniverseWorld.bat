@ECHO OFF
title WorldServer.exe || by: AzerothUniverseCore
color 07
CLS 
ECHO Server Restart Started at %time% on %date%

:DELETE_LOGS
ECHO Deleting old log files...
DEL /F /Q "..\logs\DBErrors.log" "..\logs\Eluna.log" "..\logs\GM.log" "..\logs\Server.log"
ECHO Log files deleted.

:SERVERLOOP
worldserver.exe

:: Check if worldserver.exe exited with an error
IF ERRORLEVEL 1 (
    ECHO WorldServer crashed or stopped unexpectedly at %time% on %date%
    ECHO Restarting server in 10 seconds...
    TIMEOUT /T 10 /NOBREAK
) ELSE (
    ECHO WorldServer stopped gracefully at %time% on %date%
    ECHO Restarting server in 5 seconds...
    TIMEOUT /T 10 /NOBREAK
)

ECHO.
GOTO DELETE_LOGS
:END
