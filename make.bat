@ECHO OFF
setlocal
set PROJECT_ROOT=%~dp0
set VCPKG_ROOT=%PROJECT_ROOT%vcpkg_installed
set TRIPLET=x64-windows

set LIBUI_INC=%PROJECT_ROOT%dependencies\libui
set LIBUI_LIB=%PROJECT_ROOT%dependencies\libui\build\meson-out

@REM src\*.cpp src\common\*.cpp src\settings\*.cpp src\nayuki-qr\qrcodegen.cpp

mkdir out
if "%1"=="debug" (
    echo Compiling debug version...
    mkdir "out\debug\"
    cl /EHsc /std:c++17 src/main.cpp src\icon\icon.res /Fe:out\debug\Axon.exe /Fo:out\debug\ /I "%VCPKG_ROOT%\%TRIPLET%\include" /I "%LIBUI_INC%" /link /LIBPATH:"%VCPKG_ROOT%\%TRIPLET%\lib" /LIBPATH:"%LIBUI_LIB%" libui.lib
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