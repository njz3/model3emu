echo on

SET BuildMode=Release
REM Set VERSION to last git annoted tag
FOR /F %%i IN ('git describe') DO set TAG=%%i
FOR /F %%i IN ('git rev-parse --short HEAD') DO set HASH=%%i
REM SET VERSION=%TAG%-%HASH%
SET VERSION=%TAG%-x64

ECHO %VERSION%

SET ZIP=%ProgramFiles%\7-Zip\7z.exe
SET ARCHIVE_NAME=supermodel_%VERSION%.zip
SET VS=2019\Community
SET BUILDER=%ProgramFiles(x86)%\Microsoft Visual Studio\%VS%\MSBuild\Current\Bin\MSBuild.exe
SET DEVENV=%ProgramFiles(x86)%\Microsoft Visual Studio\%VS%\Common7\IDE\devenv.com
SET OUTPUTDIR=supermodel
SET Target32=Win32\%BuildMode%\
SET Target64=x64\%BuildMode%\


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
DEL /F /Q "%Target32%\Supermodel.iobj"
DEL /F /Q "%Target32%\Supermodel.ipdb"
DEL /F /Q "%Target32%\Supermodel.log"
DEL /F /Q "%Target64%\Supermodel.iobj"
DEL /F /Q "%Target64%\Supermodel.ipdb"
DEL /F /Q "%Target64%\Supermodel.log"

SET current_path="%CD%"
RMDIR /S /Q "%OUTPUTDIR%"
MKDIR "%OUTPUTDIR%"

XCOPY /E /I "%Target64%" "%OUTPUTDIR%"
REM XCOPY "tools\fedit.exe" "%OUTPUTDIR%"
REM XCOPY "tools\vJoySetup 2.2.0 signed.exe" "%OUTPUTDIR%"
REM XCOPY /E /I "gameassets" "%OUTPUTDIR%\gameassets"
REM XCOPY /Y /I LICENSE "%OUTPUTDIR%"
REM XCOPY /Y /I README.md "%OUTPUTDIR%"
REM XCOPY /Y /I FAQ.md "%OUTPUTDIR%"
REM MOVE /Y FeederIOBoard.zip "%OUTPUTDIR%"

CD "%OUTPUTDIR%"
DEL /F /Q "..\%ARCHIVE_NAME%"
"%ZIP%" a -tzip "..\%ARCHIVE_NAME%" .
CD %current_path%
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail


:fail
exit /b 1


