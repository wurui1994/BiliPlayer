@echo off
if not exist build/BiliPlayer.exe (echo 无法找到目标文件 goto :eof)

if "%1" == "clean" goto :clean

:package
if not exist BiliPlayer mkdir BiliPlayer
REM
copy build\BiliPlayer.exe BiliPlayer
xcopy /y /s /i data BiliPlayer\data\

pushd %cd%
cd BiliPlayer
%QTDIR%\bin\windeployqt.exe BiliPlayer.exe
copy %QTDIR%\bin\libeay32.dll .
copy %QTDIR%\bin\ssleay32.dll .
REM %QTDIR%\bin\windeployqt.exe QtAVWidgets1.dll
xcopy /y /s /i "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86"
xcopy /y /s /i  "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT"
popd %cd%
pause
goto :eof

:clean
if exist BiliPlayer rmdir /s/q BiliPlayer
goto :eof