@echo off
setlocal

set configuration=debug
if "%1"=="release" (
    set configuration=release
)

set build_dir=build\%configuration%

if not exist %build_dir% (
   build.bat %configuration%
)

cd %build_dir%

if exist printccy_test.exe (
   del printccy_test.exe
)

make

if "%2"=="" (

if exist printccy_test.exe (
   gdb -batch -ex "set logging on" -ex run -ex "bt full" -ex quit --args printccy_test
)

)

cd ../..
endlocal & exit /b %BUILD_RESULT%
