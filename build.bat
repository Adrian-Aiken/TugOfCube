@echo off
rem SDK Build Script For Visual Studio
rem Created By Frank Force 2013

rem YOU MUST SET THE SDK_DIR TO POINT TO YOUR LOCAL SIFTEO SDK!
set SDK_DIR=F:\Development\Sifteo

echo ^ ^ ^_^_^_^ ^_^ ^ ^_^_^ ^_^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^_^_^_^ ^_^_^_^ ^ ^_^ ^ ^_^_
echo ^ ^/^ ^_^_^(^_^)^/^ ^_^|^ ^|^_^ ^_^_^_^ ^_^_^_^ ^ ^/^ ^_^_^|^ ^ ^ ^\^|^ ^|^/^ ^/
echo ^ ^\^_^_^ ^\^ ^|^ ^ ^_^|^ ^ ^_^/^ ^-^_^)^ ^_^ ^\^ ^\^_^_^ ^\^ ^|^)^ ^|^ ^'^ ^<
echo ^ ^|^_^_^_^/^_^|^_^|^ ^ ^\^_^_^\^_^_^_^\^_^_^_^/^ ^|^_^_^_^/^_^_^_^/^|^_^|^\^_^\
echo.

rem build the project
set PATH=%SDK_DIR%\bin;%PATH
cd source
echo Building Sifteo app...
make

rem check for error
if %errorlevel% neq 0 goto compileError

rem launch the siftulator if it builds ok
echo.
echo Sifteo build sucessfull, launching Siftulator!
echo.
siftulator sifteoApp.elf
exit 1

:compileError
echo.
echo Sifteo build error!
echo.
exit 1