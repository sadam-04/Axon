@ECHO OFF
setlocal
set PROJECT_ROOT=%~dp0
set VCPKG_ROOT=%PROJECT_ROOT%vcpkg_installed
set TRIPLET=x64-windows

set WX_DIR=C:\wx-src
set WX_LIB=%WX_DIR%\lib\vc_x64_dll
set WX_INC=%WX_DIR%\include
set WX_SETUP_INC=%WX_LIB%\mswu

@REM  src\*.cpp src\common\*.cpp src\settings\*.cpp src\nayuki-qr\qrcodegen.cpp ^
mkdir out
if "%1"=="debug" (
    echo Compiling debug version...
    mkdir "out\debug\"
    cl /EHsc /std:c++17 ^
     src/main.cpp ^
     src\icon\icon.res ^
     /Fe:out\debug\Axon.exe /Fo:out\debug\ ^
     /I"%VCPKG_ROOT%\%TRIPLET%\include" /I"%WX_INC%" /I"%WX_INC%\msvc" /I"%WX_SETUP_INC%" ^
     /D__WXMSW__ /D_UNICODE /DWXUSINGDLL ^
     /link /LIBPATH:"%WX_LIB%" /LIBPATH:"%VCPKG_ROOT%\%TRIPLET%\lib"
    xcopy src\static out\debug\static /E /I /H /Y >nul
    copy %VCPKG_ROOT%\%TRIPLET%\bin\*.dll out\debug\ >nul
)
if "%1"=="release" (
    echo Compiling release version...
    mkdir "out\release\"
    cl /EHsc /std:c++17 src\*.cpp src\common\*.cpp src\settings\*.cpp src\nayuki-qr\qrcodegen.cpp src\icon\icon.res /Fe:out\release\Axon.exe /Fo:out\release\ /I "%VCPKG_ROOT%\%TRIPLET%\include" /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /LIBPATH:"%VCPKG_ROOT%\%TRIPLET%\lib"
    xcopy src\static out\release\static /E /I /H /Y >nul
    copy %VCPKG_ROOT%\%TRIPLET%\bin\*.dll out\release\ >nul
)