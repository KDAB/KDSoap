@echo off

REM %2 is debug or release, was used as %2\%1.exe, but such subdirs don't seem to exist anymore

echo running test %1
cd %1
set OLD_PATH=%PATH%
set PATH=..\..\lib;%PATH%
%1.exe
set PATH=%OLD_PATH%
cd ..
