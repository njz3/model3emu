echo off

SET BuildMode=Release
REM Set VERSION to last git annoted tag
FOR /F %%i IN ('git describe') DO set TAG=%%i
FOR /F %%i IN ('git rev-parse --short HEAD') DO set HASH=%%i
SET VERSION=%TAG%-%HASH%

ECHO %VERSION%

SET ZIP=%ProgramFiles%\7-Zip\7z.exe
SET ARCHIVE_NAME=vJoyIOFeederSetup_%VERSION%.zip
SET VS=2019\Community
SET BUILDER=%ProgramFiles(x86)%\Microsoft Visual Studio\%VS%\MSBuild\Current\Bin\MSBuild.exe
SET DEVENV=%ProgramFiles(x86)%\Microsoft Visual Studio\%VS%\Common7\IDE\devenv.com
SET OUTPUTDIR=supermodel
REM SET Target32=x86\%BuildMode%\


:build86
echo %DATE% %TIME%: Cleaning (x86)
"%DEVENV%" /nologo Supermodel.sln /Project Supermodel /Clean "%BuildMode%|Win32"
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail

echo %DATE% %TIME%: Building (x86)
"%DEVENV%" /nologo Supermodel.sln /Project Supermodel /Build "%BuildMode%|Win32"
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail

:build64
echo %DATE% %TIME%: Cleaning (x64)
"%DEVENV%" /nologo Supermodel.sln /Project Supermodel /Clean "%BuildMode%|x64"
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail

echo %DATE% %TIME%: Building (x64)
"%DEVENV%" /nologo Supermodel.sln /Project Supermodel /Build "%BuildMode%|x64"
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail

:installer
REM SET current_path="%CD%"
REM set current_drive=%cd:~0,2%
REM CD "%ProgramFiles(x86)%\Microsoft Visual Studio\%VS%\Common7\IDE\CommonExtensions\Microsoft\VSI\DisableOutOfProcBuild"
REM C:
REM CALL DisableOutOfProcBuild.exe
REM set BUILD_STATUS=%ERRORLEVEL%
REM if not %BUILD_STATUS%==0 goto fail
REM
REM CD "%current_path%"
REM %current_drive%
REM "%DEVENV%" vJoyIOFeederSetup.sln /build "Release"

:archive
DEL /F /Q "Win32\Release\Supermodel.iobj"
DEL /F /Q "Win32\Release\Supermodel.ipdb"
DEL /F /Q "x64\Release\Supermodel.iobj"
DEL /F /Q "x64\Release\Supermodel.ipdb"

REM SET current_path="%CD%"
REM RMDIR /S /Q "%OUTPUTDIR%"
REM MKDIR "%OUTPUTDIR%"

REM XCOPY /E /I "%Target64%" "%OUTPUTDIR%"
REM XCOPY "tools\fedit.exe" "%OUTPUTDIR%"
REM XCOPY "tools\vJoySetup 2.2.0 signed.exe" "%OUTPUTDIR%"
REM XCOPY /E /I "gameassets" "%OUTPUTDIR%\gameassets"
REM XCOPY /Y /I LICENSE "%OUTPUTDIR%"
REM XCOPY /Y /I README.md "%OUTPUTDIR%"
REM XCOPY /Y /I FAQ.md "%OUTPUTDIR%"
REM MOVE /Y FeederIOBoard.zip "%OUTPUTDIR%"

REM CD "%OUTPUTDIR%"
REM DEL /F /Q "..\%ARCHIVE_NAME%"
REM "%ZIP%" a -tzip "..\%ARCHIVE_NAME%" .
REM CD %current_path%
REM set BUILD_STATUS=%ERRORLEVEL%
REM if not %BUILD_STATUS%==0 goto fail

:fail
exit /b 1


